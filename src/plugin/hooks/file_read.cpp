#include "../escape_tool.h"
#include "../iat_hook.h"
#include "../plugin_64.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace FileRead {

    struct HandleState {
        std::vector<char> pendingData;
        size_t            readOffset = 0;
    };

    static std::unordered_map<HANDLE, HandleState> trackedHandles;
    static CRITICAL_SECTION                        handleLock;

    // Cached game directory prefix for vanilla file detection
    static std::wstring gameDirPrefix;

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

        if (h != INVALID_HANDLE_VALUE && isTextFile(lpFileName)) {
            // Only track files opened for reading
            if ((dwDesiredAccess & GENERIC_READ) && !(dwDesiredAccess & GENERIC_WRITE)) {
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
        if (it->second.pendingData.empty() && it->second.readOffset == 0) {
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

    void Init(const RunOptions& options) {
        if (!options.autoUtf8Conversion)
            return;

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

        origReadFile = (decltype(origReadFile))IATHook::Hook(exeModule, "kernel32.dll", "ReadFile",
                                                             (void*)hookedReadFile);

        origCloseHandle = (decltype(origCloseHandle))IATHook::Hook(
            exeModule, "kernel32.dll", "CloseHandle", (void*)hookedCloseHandle);
    }

} // namespace FileRead
