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
        size_t            readOffset    = 0;
        bool              isChecksumHook = false; // true = serve empty content
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

    static bool isTextFile(const wchar_t* path) {
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
        return lext == L".yml" || lext == L".txt";
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

    static bool needsUtf8Conversion(const char* buf, size_t len) {
        bool hasMultibyte = false;

        for (size_t i = 0; i < len; i++) {
            unsigned char c = (unsigned char)buf[i];

            // Check for escape markers — already in escape encoding
            if (c >= 0x10 && c <= 0x13)
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
                    continue; // Invalid UTF-8 leading byte

                if (i + seqLen > len)
                    continue; // Not enough bytes

                bool valid = true;
                for (size_t j = 1; j < seqLen; j++) {
                    if (((unsigned char)buf[i + j] & 0xC0) != 0x80) {
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    hasMultibyte = true;
                    i += seqLen - 1; // Skip continuation bytes
                }
            }
        }

        return hasMultibyte;
    }

    static bool convertUtf8ToEscaped(const char* input, size_t inputLen,
                                     std::vector<char>& output) {
        // Skip BOM if present
        const char* src    = input;
        size_t      srcLen = inputLen;
        if (srcLen >= 3 && (unsigned char)src[0] == 0xEF && (unsigned char)src[1] == 0xBB &&
            (unsigned char)src[2] == 0xBF) {
            src += 3;
            srcLen -= 3;
        }

        // Need null-terminated string for convertTextToWideText
        std::string nullTerminated(src, srcLen);

        std::wstring wideText = convertTextToWideText(nullTerminated.c_str());
        if (wideText.empty())
            return false;

        std::string escapedText = convertWideTextToEscapedText(wideText.c_str());
        if (escapedText.empty())
            return false;

        output.assign(escapedText.begin(), escapedText.end());

        return true;
    }

    // Read entire file content from current position to end.
    // Returns empty vector on failure.
    static std::vector<char> readEntireFile(HANDLE hFile) {
        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart == 0)
            return {};

        size_t            totalSize = (size_t)fileSize.QuadPart;
        std::vector<char> content(totalSize);

        // Seek to beginning
        LARGE_INTEGER zero = {};
        SetFilePointerEx(hFile, zero, nullptr, FILE_BEGIN);

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

    static HANDLE WINAPI hookedCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                                           DWORD                 dwShareMode,
                                           LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                           DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                                           HANDLE hTemplateFile) {
        HANDLE h = origCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
                                   dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

        if (h != INVALID_HANDLE_VALUE && (dwDesiredAccess & GENERIC_READ) &&
            !(dwDesiredAccess & GENERIC_WRITE)) {

            // Checksum override: intercept checksum_manifest.txt and serve empty content
            if (!checksumOverride.empty() && isChecksumManifest(lpFileName)) {
                HandleState state;
                state.isChecksumHook = true;
                // Empty pendingData with readOffset=1 signals "serve empty content"
                // (readOffset=1 so hookedReadFile won't try to read the actual file)
                state.pendingData = {'\n'}; // minimal valid content (single newline)
                state.readOffset  = 0;
                EnterCriticalSection(&handleLock);
                trackedHandles[h] = std::move(state);
                LeaveCriticalSection(&handleLock);
                return h;
            }

            if (utf8ConversionEnabled && isTextFile(lpFileName)) {
                // Skip vanilla files — they should not be converted
                if (!isVanillaFile(lpFileName)) {
                    EnterCriticalSection(&handleLock);
                    trackedHandles[h] = {};
                    LeaveCriticalSection(&handleLock);
                }
            }
        }

        return h;
    }

    static BOOL WINAPI hookedReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                                      LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
        EnterCriticalSection(&handleLock);
        auto it = trackedHandles.find(hFile);
        if (it == trackedHandles.end()) {
            LeaveCriticalSection(&handleLock);
            return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
                                lpOverlapped);
        }

        // First read on this handle: load and convert the file
        // (skip for checksum hook handles — their data is already set)
        if (!it->second.isChecksumHook && it->second.pendingData.empty() &&
            it->second.readOffset == 0) {
            // Release lock during file I/O and conversion
            LeaveCriticalSection(&handleLock);

            std::vector<char> fileContent = readEntireFile(hFile);

            if (fileContent.empty()) {
                // Failed to read — untrack and fall back
                EnterCriticalSection(&handleLock);
                trackedHandles.erase(hFile);
                LeaveCriticalSection(&handleLock);
                LARGE_INTEGER zero = {};
                SetFilePointerEx(hFile, zero, nullptr, FILE_BEGIN);
                return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
                                    lpOverlapped);
            }

            if (!needsUtf8Conversion(fileContent.data(), fileContent.size())) {
                // No conversion needed — untrack and let original ReadFile handle it
                EnterCriticalSection(&handleLock);
                trackedHandles.erase(hFile);
                LeaveCriticalSection(&handleLock);
                LARGE_INTEGER zero = {};
                SetFilePointerEx(hFile, zero, nullptr, FILE_BEGIN);
                return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
                                    lpOverlapped);
            }

            std::vector<char> converted;
            if (!convertUtf8ToEscaped(fileContent.data(), fileContent.size(), converted)) {
                // Conversion failed — untrack and fall back with original data
                EnterCriticalSection(&handleLock);
                trackedHandles.erase(hFile);
                LeaveCriticalSection(&handleLock);
                LARGE_INTEGER zero = {};
                SetFilePointerEx(hFile, zero, nullptr, FILE_BEGIN);
                return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
                                    lpOverlapped);
            }

            EnterCriticalSection(&handleLock);
            // Re-check: handle may have been closed/erased while lock was released
            it = trackedHandles.find(hFile);
            if (it == trackedHandles.end()) {
                LeaveCriticalSection(&handleLock);
                return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
                                    lpOverlapped);
            }
            it->second.pendingData = std::move(converted);
            it->second.readOffset  = 0;
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

    static BOOL WINAPI hookedCloseHandle(HANDLE hObject) {
        EnterCriticalSection(&handleLock);
        trackedHandles.erase(hObject);
        LeaveCriticalSection(&handleLock);

        return origCloseHandle(hObject);
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
