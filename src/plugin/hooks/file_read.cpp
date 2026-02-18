#include "../byte_pattern.h"
#include "../escape_tool.h"
#include "../iat_hook.h"
#include "../injector.h"
#include "../plugin_64.h"
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

    static bool needsUtf8Conversion(const char* buf, size_t len, bool isTxtFile) {
        // UTF-8 BOM: check content after BOM for escape markers to distinguish
        // pre-encoded files (BOM + escape sequences) from raw UTF-8 files (BOM + UTF-8).
        if (len >= 3 && (unsigned char)buf[0] == 0xEF &&
            (unsigned char)buf[1] == 0xBB && (unsigned char)buf[2] == 0xBF) {
            // Scan for escape markers and multibyte content in one pass
            bool hasHighByte = false;
            for (size_t i = 3; i < len; i++) {
                unsigned char c = (unsigned char)buf[i];
                if (c >= 0x10 && c <= 0x13)
                    return false; // Escape marker found → already pre-encoded
                if (c >= 0x80)
                    hasHighByte = true;
            }
            // BOM + no escape markers + has multibyte → genuine UTF-8, needs conversion
            // BOM + no escape markers + pure ASCII → no conversion needed
            return hasHighByte;
        }

        // No BOM: use heuristics to detect UTF-8 multibyte content
        bool hasMultibyte = false;

        for (size_t i = 0; i < len; i++) {
            unsigned char c = (unsigned char)buf[i];

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
        if (trackedHandles.empty())
            return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
                                lpOverlapped);

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
        if (trackedHandles.empty())
            return origSetFilePointerEx(hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);

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
        if (!trackedHandles.empty()) {
            EnterCriticalSection(&handleLock);
            trackedHandles.erase(hObject);
            LeaveCriticalSection(&handleLock);
        }

        return origCloseHandle(hObject);
    }

    static BOOL WINAPI hookedGetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) {
        if (trackedHandles.empty())
            return origGetFileSizeEx(hFile, lpFileSize);

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
        if (trackedHandles.empty())
            return origGetFileSize(hFile, lpFileSizeHigh);

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

    // Checksum spoofing: the game compares a computed checksum against a hardcoded
    // vanilla value at 2 call sites. We patch the comparison result to always
    // indicate "equal", bypassing mod detection for achievements and multiplayer.
    static const char* vanillaChecksum = nullptr;

    void Init(const RunOptions& options) {
        utf8ConversionEnabled = options.autoUtf8Conversion;

        // Install IAT hooks for UTF-8 auto conversion
        if (options.autoUtf8Conversion) {
            InitializeCriticalSection(&handleLock);

            // Cache game directory prefix for vanilla file detection
            wchar_t exePath[MAX_PATH];
            if (GetModuleFileNameW(nullptr, exePath, MAX_PATH)) {
                wchar_t* lastSlash = wcsrchr(exePath, L'\\');
                if (lastSlash) {
                    gameDirPrefix.assign(exePath, lastSlash + 1);
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
        }

        // Checksum spoof: patch "TEST EAX,EAX / SETZ BL" after the compare CALL
        // to "MOV BL,1 / NOP / NOP / NOP", forcing the result to always be "equal".
        if (options.achievementUnlock) {
            BytePattern::temp_instance()
                .find_pattern("48 8B 12 48 8D 0D ? ? ? ? E8 ? ? ? ? 85 C0 0F 94 C3");

            const size_t expected = 2;
            size_t count = BytePattern::temp_instance().count();

            if (count == 0) {
                BytePattern::LoggingInfo("[checksum] Pattern not found!\n");
            } else {
                if (count != expected) {
                    BytePattern::LoggingInfo(
                        "[checksum] WARNING: expected " + std::to_string(expected) +
                        " matches, found " + std::to_string(count) + "\n");
                }

                uintptr_t firstMatch = BytePattern::temp_instance().get_first().address();

                // Resolve vanilla checksum address for logging
                vanillaChecksum = reinterpret_cast<const char*>(
                    Injector::GetBranchDestination(firstMatch + 3, true));

                const uint8_t expectedBytes[] = { 0x85, 0xC0, 0x0F, 0x94, 0xC3 };
                size_t patched = 0;
                for (size_t i = 0; i < count; i++) {
                    uintptr_t addr = BytePattern::temp_instance().get(i).address();
                    uintptr_t patchAddr = addr + 15;

                    // Verify target bytes before patching
                    bool match = true;
                    for (int j = 0; j < 5; j++) {
                        if (Injector::ReadMemory<uint8_t>(patchAddr + j) != expectedBytes[j]) {
                            match = false;
                            break;
                        }
                    }
                    if (!match) {
                        BytePattern::LoggingInfo(
                            "[checksum] Skipped site " + std::to_string(i) +
                            ": unexpected bytes at patch target\n");
                        continue;
                    }

                    // "85 C0 0F 94 C3" = TEST EAX,EAX / SETZ BL (5 bytes)
                    // → "B3 01 90 90 90" = MOV BL,1 / NOP / NOP / NOP
                    Injector::WriteMemory<uint8_t>(patchAddr,     0xB3, true);
                    Injector::WriteMemory<uint8_t>(patchAddr + 1, 0x01, true);
                    Injector::WriteMemory<uint8_t>(patchAddr + 2, 0x90, true);
                    Injector::WriteMemory<uint8_t>(patchAddr + 3, 0x90, true);
                    Injector::WriteMemory<uint8_t>(patchAddr + 4, 0x90, true);
                    patched++;
                }

                BytePattern::LoggingInfo(
                    "[checksum] Patched " + std::to_string(patched) + "/" +
                    std::to_string(count) + " compare sites" +
                    (vanillaChecksum ? ", vanilla checksum: " + std::string(vanillaChecksum) : "") +
                    "\n");
            }
        }
    }

} // namespace FileRead
