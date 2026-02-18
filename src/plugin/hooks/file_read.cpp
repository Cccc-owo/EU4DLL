#include "../byte_pattern.h"
#include "../escape_tool.h"
#include "../iat_hook.h"
#include "../plugin_64.h"
#include <csetjmp>
#include <string>
#include <unordered_map>
#include <vector>

namespace FileRead {

    struct HandleState {
        std::vector<char> pendingData;
        size_t            readOffset = 0;
        bool              converted  = false; // true = pendingData holds converted content
    };

    static std::unordered_map<HANDLE, HandleState> trackedHandles;
    static CRITICAL_SECTION                        handleLock;

    // Cached game directory prefix for vanilla file detection
    static std::wstring gameDirPrefix;

    // Checksum override config (empty = disabled)
    static std::string checksumOverride;

    // Whether UTF-8 auto conversion is enabled
    static bool utf8ConversionEnabled = false;

    static decltype(&CreateFileW) origCreateFileW = nullptr;
    static decltype(&ReadFile)    origReadFile    = nullptr;
    static decltype(&CloseHandle) origCloseHandle = nullptr;

    using GetFileSizeExPtr    = BOOL(WINAPI*)(HANDLE, PLARGE_INTEGER);
    using SetFilePointerExPtr = BOOL(WINAPI*)(HANDLE, LARGE_INTEGER, PLARGE_INTEGER, DWORD);
    using GetFileSizePtr      = DWORD(WINAPI*)(HANDLE, LPDWORD);
    static GetFileSizeExPtr    origGetFileSizeEx    = nullptr;
    static SetFilePointerExPtr origSetFilePointerEx = nullptr;
    static GetFileSizePtr      origGetFileSize      = nullptr;

    static bool isTextFile(const wchar_t* path, bool* isTxt = nullptr) {
        if (!path)
            return false;

        std::wstring_view sv(path);
        auto              dot = sv.rfind(L'.');
        if (dot == std::wstring_view::npos)
            return false;

        auto ext = sv.substr(dot);

        if (ext.size() < 3 || ext.size() > 5)
            return false;

        // Case-insensitive comparison for common text extensions
        wchar_t lower[6] = {};
        for (size_t i = 0; i < ext.size() && i < 5; i++)
            lower[i] = (ext[i] >= L'A' && ext[i] <= L'Z') ? ext[i] + 32 : ext[i];

        std::wstring_view lext(lower, ext.size());

        if (lext == L".yml" || lext == L".txt") {
            if (isTxt)
                *isTxt = (lext == L".txt");
            return true;
        }
        return false;
    }

    static bool isVanillaFile(const wchar_t* path) {
        if (gameDirPrefix.empty() || !path)
            return false;
        size_t prefixLen = gameDirPrefix.size();
        size_t pathLen   = wcslen(path);
        if (pathLen < prefixLen)
            return false;
        return _wcsnicmp(path, gameDirPrefix.c_str(), prefixLen) == 0;
    }

    static bool isChecksumManifest(const wchar_t* path) {
        if (!path)
            return false;
        // Match filename "checksum_manifest.txt" at end of path
        const wchar_t* name   = L"checksum_manifest.txt";
        size_t         nameLen = wcslen(name);
        size_t         pathLen = wcslen(path);
        if (pathLen < nameLen)
            return false;
        const wchar_t* tail = path + pathLen - nameLen;
        if (_wcsicmp(tail, name) != 0)
            return false;
        // Ensure it's preceded by a path separator or is the entire path
        if (tail != path && tail[-1] != L'\\' && tail[-1] != L'/')
            return false;
        return true;
    }

    static bool needsUtf8Conversion(const char* buf, size_t len, bool isTxtFile) {
        // UTF-8 BOM: file is definitively UTF-8, always convert
        if (len >= 3 && (unsigned char)buf[0] == 0xEF &&
            (unsigned char)buf[1] == 0xBB && (unsigned char)buf[2] == 0xBF)
            return true;

        bool hasMultibyte = false;

        for (size_t i = 0; i < len; i++) {
            unsigned char c = (unsigned char)buf[i];

            // Escape markers 0x10-0x13: already in game's internal encoding
            if (c >= 0x10 && c <= 0x13)
                return false;

            // CP1252 / Latin-1 detection: only for .txt files which may use these encodings.
            // Bytes 0x80-0xBF can only appear as continuation bytes (10xxxxxx) in valid
            // UTF-8, never as leading bytes. If such a byte appears outside a UTF-8
            // multibyte sequence, the file is not UTF-8.
            if (isTxtFile && c >= 0x80 && c <= 0xBF)
                return false;

            // Check for UTF-8 multibyte sequences
            if (c >= 0xC0) {
                size_t seqLen = 0;
                if ((c & 0xE0) == 0xC0)
                    seqLen = 2; // 110xxxxx
                else if ((c & 0xF0) == 0xE0)
                    seqLen = 3; // 1110xxxx
                else if ((c & 0xF8) == 0xF0)
                    seqLen = 4; // 11110xxx
                else
                    return false; // Invalid UTF-8 leading byte — not UTF-8

                if (i + seqLen > len)
                    return false; // Truncated sequence — not valid UTF-8

                bool valid = true;
                for (size_t j = 1; j < seqLen; j++) {
                    if (((unsigned char)buf[i + j] & 0xC0) != 0x80) {
                        valid = false;
                        break;
                    }
                }

                if (!valid)
                    return false; // Broken sequence — not valid UTF-8

                hasMultibyte = true;
                i += seqLen - 1; // Skip continuation bytes
            }
        }

        return hasMultibyte;
    }

    static bool convertUtf8ToEscaped(const char* input, size_t inputLen,
                                     std::vector<char>& output) {
        // Skip BOM if present, but remember it so we can restore it
        const char* src    = input;
        size_t      srcLen = inputLen;
        bool        hasBom = false;
        if (srcLen >= 3 && (unsigned char)src[0] == 0xEF && (unsigned char)src[1] == 0xBB &&
            (unsigned char)src[2] == 0xBF) {
            hasBom = true;
            src += 3;
            srcLen -= 3;
        }

        // Need null-terminated string for convertTextToWideText
        std::string nullTerminated(src, srcLen);

        std::wstring wideText = convertTextToWideText(nullTerminated.c_str());
        if (wideText.empty())
            return false;

        std::string escapedText = convertWideTextToEscapedText(wideText.c_str(), true);
        if (escapedText.empty())
            return false;

        // The escape encoding produces raw bytes (0x00-0xFF) that are not valid UTF-8.
        // The game reads UTF-8 files, so each escaped byte must be re-encoded:
        //   byte 0x00-0x7F -> output as-is (single UTF-8 byte)
        //   byte 0x80-0xFF -> map via CP1252ToUCS2 to Unicode codepoint -> UTF-8
        std::string utf8Result;
        utf8Result.reserve(escapedText.size() * 2);

        for (size_t i = 0; i < escapedText.size(); i++) {
            unsigned char b = (unsigned char)escapedText[i];
            wchar_t       cp = CP1252ToUCS2(b);

            if (cp < 0x80) {
                utf8Result += (char)cp;
            } else if (cp < 0x800) {
                utf8Result += (char)(0xC0 | (cp >> 6));
                utf8Result += (char)(0x80 | (cp & 0x3F));
            } else {
                utf8Result += (char)(0xE0 | (cp >> 12));
                utf8Result += (char)(0x80 | ((cp >> 6) & 0x3F));
                utf8Result += (char)(0x80 | (cp & 0x3F));
            }
        }

        // Restore BOM — EU4's YML parser requires it
        if (hasBom) {
            output.reserve(3 + utf8Result.size());
            output.push_back((char)0xEF);
            output.push_back((char)0xBB);
            output.push_back((char)0xBF);
            output.insert(output.end(), utf8Result.begin(), utf8Result.end());
        } else {
            output.assign(utf8Result.begin(), utf8Result.end());
        }

        return true;
    }

    // Helper to seek a file handle back to the beginning using the original API.
    static void seekToBeginning(HANDLE hFile) {
        LARGE_INTEGER zero = {};
        auto seekFunc = origSetFilePointerEx ? origSetFilePointerEx
                                             : (SetFilePointerExPtr)&SetFilePointerEx;
        seekFunc(hFile, zero, nullptr, FILE_BEGIN);
    }

    // Read entire file content from beginning to end.
    // Returns empty vector on failure.
    static std::vector<char> readEntireFile(HANDLE hFile) {
        LARGE_INTEGER fileSize;
        auto getSizeFunc = origGetFileSizeEx ? origGetFileSizeEx
                                            : (GetFileSizeExPtr)&GetFileSizeEx;
        if (!getSizeFunc(hFile, &fileSize) || fileSize.QuadPart == 0)
            return {};

        size_t            totalSize = (size_t)fileSize.QuadPart;
        std::vector<char> content(totalSize);

        seekToBeginning(hFile);

        size_t totalRead = 0;
        while (totalRead < totalSize) {
            DWORD toRead    = (DWORD)std::min(totalSize - totalRead, (size_t)0x7FFFFFFF);
            DWORD bytesRead = 0;
            if (!origReadFile(hFile, content.data() + totalRead, toRead, &bytesRead, nullptr) ||
                bytesRead == 0)
                break;
            totalRead += bytesRead;
        }

        if (totalRead == 0)
            return {};

        content.resize(totalRead);
        return content;
    }

    // Untrack a handle: seek back to beginning and remove from map.
    static void untrackHandle(HANDLE hFile) {
        seekToBeginning(hFile);
        EnterCriticalSection(&handleLock);
        trackedHandles.erase(hFile);
        LeaveCriticalSection(&handleLock);
    }

    // Eagerly read and convert a file right after opening.
    // If conversion succeeds, populates state.pendingData and sets state.converted = true.
    // If the file doesn't need conversion, the handle is untracked.
    // Must be called WITHOUT handleLock held.
    static void eagerConvert(HANDLE hFile, bool isTxtFile) {
        std::vector<char> fileContent = readEntireFile(hFile);

        if (fileContent.empty()) {
            untrackHandle(hFile);
            return;
        }

        if (!needsUtf8Conversion(fileContent.data(), fileContent.size(), isTxtFile)) {
            untrackHandle(hFile);
            return;
        }

        std::vector<char> converted;
        if (!convertUtf8ToEscaped(fileContent.data(), fileContent.size(), converted)) {
            untrackHandle(hFile);
            return;
        }

        EnterCriticalSection(&handleLock);
        auto it = trackedHandles.find(hFile);
        if (it != trackedHandles.end()) {
            it->second.pendingData = std::move(converted);
            it->second.readOffset  = 0;
            it->second.converted   = true;
        }
        LeaveCriticalSection(&handleLock);

        seekToBeginning(hFile);
    }

    static HANDLE WINAPI hookedCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                                           DWORD                 dwShareMode,
                                           LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                           DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                                           HANDLE hTemplateFile) {
        HANDLE h = origCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
                                   dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

        if (h != INVALID_HANDLE_VALUE && (dwDesiredAccess & GENERIC_READ) &&
            !(dwDesiredAccess & GENERIC_WRITE)) {

            bool txtFlag = false;

            // Checksum override: intercept checksum_manifest.txt and serve empty content
            if (!checksumOverride.empty() && isChecksumManifest(lpFileName)) {
                HandleState state;
                state.converted   = true;
                state.pendingData = {'\n'}; // minimal valid content (single newline)
                state.readOffset  = 0;
                EnterCriticalSection(&handleLock);
                trackedHandles[h] = std::move(state);
                LeaveCriticalSection(&handleLock);
                return h;
            }

            if (utf8ConversionEnabled && isTextFile(lpFileName, &txtFlag)) {
                // Skip vanilla files — they should not be converted
                if (!isVanillaFile(lpFileName)) {
                    HandleState state;
                    EnterCriticalSection(&handleLock);
                    trackedHandles[h] = std::move(state);
                    LeaveCriticalSection(&handleLock);

                    // Eagerly read and convert now so that GetFileSize/GetFileSizeEx
                    // returns the converted size before the first ReadFile
                    eagerConvert(h, txtFlag);
                }
            }
        }

        return h;
    }

    static BOOL WINAPI hookedReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                                      LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
        EnterCriticalSection(&handleLock);
        auto it = trackedHandles.find(hFile);
        if (it == trackedHandles.end() || !it->second.converted) {
            LeaveCriticalSection(&handleLock);
            return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
                                lpOverlapped);
        }

        // Serve data from shadow buffer
        HandleState& state     = it->second;
        size_t       remaining = state.pendingData.size() - state.readOffset;
        DWORD        toCopy    = (DWORD)std::min((size_t)nNumberOfBytesToRead, remaining);

        if (toCopy > 0) {
            memcpy(lpBuffer, state.pendingData.data() + state.readOffset, toCopy);
            state.readOffset += toCopy;
        }

        if (lpNumberOfBytesRead)
            *lpNumberOfBytesRead = toCopy;

        LeaveCriticalSection(&handleLock);
        return TRUE;
    }

    static BOOL WINAPI hookedSetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove,
                                               PLARGE_INTEGER lpNewFilePointer,
                                               DWORD          dwMoveMethod) {
        EnterCriticalSection(&handleLock);
        auto it = trackedHandles.find(hFile);
        if (it != trackedHandles.end() && it->second.converted) {
            HandleState& state   = it->second;
            LONGLONG     dataLen = (LONGLONG)state.pendingData.size();
            LONGLONG     newPos  = 0;

            switch (dwMoveMethod) {
                case FILE_BEGIN:   newPos = liDistanceToMove.QuadPart; break;
                case FILE_CURRENT: newPos = (LONGLONG)state.readOffset + liDistanceToMove.QuadPart; break;
                case FILE_END:     newPos = dataLen + liDistanceToMove.QuadPart; break;
                default:
                    LeaveCriticalSection(&handleLock);
                    SetLastError(ERROR_INVALID_PARAMETER);
                    return FALSE;
            }

            if (newPos < 0)
                newPos = 0;
            if (newPos > dataLen)
                newPos = dataLen;

            state.readOffset = (size_t)newPos;

            if (lpNewFilePointer)
                lpNewFilePointer->QuadPart = newPos;

            LeaveCriticalSection(&handleLock);
            return TRUE;
        }
        LeaveCriticalSection(&handleLock);
        return origSetFilePointerEx(hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
    }

    static BOOL WINAPI hookedCloseHandle(HANDLE hObject) {
        EnterCriticalSection(&handleLock);
        trackedHandles.erase(hObject);
        LeaveCriticalSection(&handleLock);

        return origCloseHandle(hObject);
    }

    static BOOL WINAPI hookedGetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) {
        EnterCriticalSection(&handleLock);
        auto it = trackedHandles.find(hFile);
        if (it != trackedHandles.end() && it->second.converted) {
            if (lpFileSize)
                lpFileSize->QuadPart = (LONGLONG)it->second.pendingData.size();
            LeaveCriticalSection(&handleLock);
            return TRUE;
        }
        LeaveCriticalSection(&handleLock);
        return origGetFileSizeEx(hFile, lpFileSize);
    }

    static DWORD WINAPI hookedGetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) {
        EnterCriticalSection(&handleLock);
        auto it = trackedHandles.find(hFile);
        if (it != trackedHandles.end() && it->second.converted) {
            LONGLONG convSize = (LONGLONG)it->second.pendingData.size();
            LeaveCriticalSection(&handleLock);
            if (lpFileSizeHigh)
                *lpFileSizeHigh = (DWORD)(convSize >> 32);
            return (DWORD)(convSize & 0xFFFFFFFF);
        }
        LeaveCriticalSection(&handleLock);
        return origGetFileSize(hFile, lpFileSizeHigh);
    }

    // Known checksum when manifest is emptied — used as search target
    static constexpr const char* EMPTY_MANIFEST_CHECKSUM = "cfda";

    static bool isHexChar(char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    // VEH-based crash protection for memory scanning
    static thread_local jmp_buf scanJmpBuf;
    static thread_local DWORD   scanThreadId = 0;

    static LONG WINAPI scanVEH(EXCEPTION_POINTERS* ep) {
        if (GetCurrentThreadId() == scanThreadId &&
            ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
            longjmp(scanJmpBuf, 1);
        }
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Scan one memory region for "cfda" patterns and patch them.
    // Returns number of patches applied.
    static int scanAndPatchRegion(char* base, SIZE_T size) {
        int patched = 0;

        // Re-verify the memory is still valid before scanning
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(base, &mbi, sizeof(mbi)) ||
            mbi.State != MEM_COMMIT || mbi.Protect != PAGE_READWRITE)
            return 0;

        SIZE_T safeSize = (SIZE_T)((char*)mbi.BaseAddress + mbi.RegionSize - base);
        if (safeSize > size)
            safeSize = size;

        // Use setjmp to recover from access violations during scan
        if (setjmp(scanJmpBuf) != 0) {
            // Jumped back from VEH — memory became invalid, abort this region
            return patched;
        }

        for (SIZE_T off = 0; off + 5 <= safeSize; off++) {
            if (base[off] != 'c')
                continue;
            if (memcmp(base + off, EMPTY_MANIFEST_CHECKSUM, 4) != 0)
                continue;

            // Pattern 1: hash tail + "cfda\0"
            bool isHashChecksum = false;
            if (base[off + 4] == '\0' && off >= 8) {
                isHashChecksum = true;
                for (int j = 1; j <= 8; j++) {
                    if (!isHexChar(base[off - j])) {
                        isHashChecksum = false;
                        break;
                    }
                }
            }

            // Pattern 2: "(cfda)"
            bool isUIDisplay = false;
            if (off >= 1 && base[off - 1] == '(' &&
                off + 4 < safeSize && base[off + 4] == ')') {
                isUIDisplay = true;
            }

            if (isHashChecksum || isUIDisplay) {
                char addrStr[32];
                snprintf(addrStr, sizeof(addrStr), "%p", base + off);
                BytePattern::LoggingInfo(
                    "[checksum] Patching at " + std::string(addrStr) +
                    (isHashChecksum ? " [hash]" : " [UI]") + "\n");

                memcpy(base + off, checksumOverride.c_str(), 4);
                patched++;
            }
        }

        return patched;
    }

    static DWORD WINAPI checksumPatchThread(LPVOID) {
        Sleep(10000);

        BytePattern::LoggingInfo("[checksum] Patch thread started, polling for cfda...\n");

        // Register VEH for crash protection during memory scanning
        scanThreadId = GetCurrentThreadId();
        PVOID vehHandle = AddVectoredExceptionHandler(1, scanVEH);

        int totalPatched  = 0;
        int roundsWithHit = 0;
        int roundsNoHit   = 0;

        for (int round = 0; round < 60; round++) {
            if (round > 0)
                Sleep(5000);

            int patchedThisRound = 0;

            MEMORY_BASIC_INFORMATION mbi;
            const unsigned char*     addr = nullptr;

            while (VirtualQuery(addr, &mbi, sizeof(mbi))) {
                if (mbi.State == MEM_COMMIT && mbi.Protect == PAGE_READWRITE) {
                    patchedThisRound += scanAndPatchRegion(
                        reinterpret_cast<char*>(mbi.BaseAddress), mbi.RegionSize);
                }

                addr = reinterpret_cast<const unsigned char*>(mbi.BaseAddress) + mbi.RegionSize;
                if (addr < reinterpret_cast<const unsigned char*>(mbi.BaseAddress))
                    break;
            }

            if (patchedThisRound > 0) {
                roundsWithHit++;
                roundsNoHit = 0;
                totalPatched += patchedThisRound;
                BytePattern::LoggingInfo("[checksum] Round " + std::to_string(round) +
                                        ": patched " + std::to_string(patchedThisRound) +
                                        " (total " + std::to_string(totalPatched) + ")\n");
            } else if (roundsWithHit > 0) {
                roundsNoHit++;
                if (roundsNoHit >= 5) {
                    BytePattern::LoggingInfo(
                        "[checksum] No new matches for 5 rounds, stopping.\n");
                    break;
                }
            }
        }

        if (vehHandle)
            RemoveVectoredExceptionHandler(vehHandle);

        BytePattern::LoggingInfo("[checksum] Thread done. Total patched: " +
                                std::to_string(totalPatched) + "\n");
        return 0;
    }

    void Init(const RunOptions& options) {
        checksumOverride      = options.checksumOverride;
        utf8ConversionEnabled = options.autoUtf8Conversion;

        bool needIATHooks = options.autoUtf8Conversion || !checksumOverride.empty();

        // Install IAT hooks if UTF-8 conversion or checksum override needs them
        if (needIATHooks) {
            InitializeCriticalSection(&handleLock);

            // Cache game directory prefix for vanilla file detection
            if (options.autoUtf8Conversion) {
                wchar_t exePath[MAX_PATH];
                if (GetModuleFileNameW(nullptr, exePath, MAX_PATH)) {
                    wchar_t* lastSlash = wcsrchr(exePath, L'\\');
                    if (lastSlash) {
                        gameDirPrefix.assign(exePath, lastSlash + 1);
                    }
                }
            }

            HMODULE exeModule = GetModuleHandle(nullptr);

            origCreateFileW = (decltype(origCreateFileW))IATHook::Hook(
                exeModule, "kernel32.dll", "CreateFileW", (void*)hookedCreateFileW);

            origReadFile = (decltype(origReadFile))IATHook::Hook(
                exeModule, "kernel32.dll", "ReadFile", (void*)hookedReadFile);

            origCloseHandle = (decltype(origCloseHandle))IATHook::Hook(
                exeModule, "kernel32.dll", "CloseHandle", (void*)hookedCloseHandle);

            origGetFileSizeEx = (GetFileSizeExPtr)IATHook::Hook(
                exeModule, "kernel32.dll", "GetFileSizeEx", (void*)hookedGetFileSizeEx);

            origSetFilePointerEx = (SetFilePointerExPtr)IATHook::Hook(
                exeModule, "kernel32.dll", "SetFilePointerEx", (void*)hookedSetFilePointerEx);

            origGetFileSize = (GetFileSizePtr)IATHook::Hook(
                exeModule, "kernel32.dll", "GetFileSize", (void*)hookedGetFileSize);

            // Launch background thread to patch checksum in memory after game computes it
            if (!checksumOverride.empty() && checksumOverride.size() == 4) {
                CreateThread(nullptr, 0, checksumPatchThread, nullptr, 0, nullptr);
            }
        }

        // Achievement unlock: patch checksum validation independently
        if (options.achievementUnlock) {
            // Patch achievement checksum validation: TEST EAX,EAX -> XOR EAX,EAX
            // This makes the checksum comparison always return "match",
            // enabling achievements with modded checksums.
            // Pattern: 48 8B 12 48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 85 C0 0F 94 C3
            BytePattern::temp_instance()
                .find_pattern("48 8B 12 48 8D 0D ? ? ? ? E8 ? ? ? ? 85 C0 0F 94 C3");

            if (BytePattern::temp_instance().count() > 0) {
                for (size_t i = 0; i < BytePattern::temp_instance().count(); i++) {
                    auto addr = BytePattern::temp_instance().get(i).address();
                    // 85 C0 is at offset +15 from pattern start
                    void* patchAddr = reinterpret_cast<void*>(addr + 15);
                    DWORD oldProtect;
                    if (VirtualProtect(patchAddr, 2, PAGE_EXECUTE_READWRITE, &oldProtect)) {
                        // 85 C0 (TEST EAX,EAX) -> 31 C0 (XOR EAX,EAX)
                        *(unsigned char*)patchAddr = 0x31;
                        VirtualProtect(patchAddr, 2, oldProtect, &oldProtect);
                        BytePattern::LoggingInfo("[achievement] Patch applied at match #" +
                                                std::to_string(i) + "\n");
                    }
                }
            } else {
                BytePattern::LoggingInfo("[achievement] Pattern not found!\n");
            }
        }
    }

} // namespace FileRead
