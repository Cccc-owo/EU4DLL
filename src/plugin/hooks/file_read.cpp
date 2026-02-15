#include "../plugin64.h"
#include "../plugin_64.h"
#include "../iat_hook.h"
#include "../escape_tool.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace FileRead {

struct HandleState {
    std::vector<char> pendingData;
    size_t readOffset;
};

static std::unordered_map<HANDLE, HandleState> trackedHandles;
static CRITICAL_SECTION handleLock;

static decltype(&CreateFileW) origCreateFileW = nullptr;
static decltype(&ReadFile) origReadFile = nullptr;
static decltype(&CloseHandle) origCloseHandle = nullptr;

static bool isTextFile(const wchar_t* path)
{
    if (!path)
        return false;

    std::wstring_view sv(path);
    auto dot = sv.rfind(L'.');
    if (dot == std::wstring_view::npos)
        return false;

    auto ext = sv.substr(dot);

    // Case-insensitive comparison for common text extensions
    if (ext.size() < 3 || ext.size() > 5)
        return false;

    wchar_t lower[6] = {};
    for (size_t i = 0; i < ext.size() && i < 5; i++)
        lower[i] = (ext[i] >= L'A' && ext[i] <= L'Z') ? ext[i] + 32 : ext[i];

    std::wstring_view lext(lower, ext.size());
    return lext == L".yml" || lext == L".txt" || lext == L".csv" || lext == L".lua";
}

static bool needsUtf8Conversion(const char* buf, size_t len)
{
    bool hasMultibyte = false;

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)buf[i];

        // Check for escape markers — already in escape encoding
        if (c >= 0x10 && c <= 0x13)
            return false;

        // Check for UTF-8 multibyte sequences
        if (c >= 0xC0) {
            size_t seqLen = 0;
            if ((c & 0xE0) == 0xC0) seqLen = 2;       // 110xxxxx
            else if ((c & 0xF0) == 0xE0) seqLen = 3;   // 1110xxxx
            else if ((c & 0xF8) == 0xF0) seqLen = 4;   // 11110xxx
            else continue; // Invalid UTF-8 leading byte

            if (i + seqLen > len)
                continue; // Not enough bytes

            bool valid = true;
            for (size_t j = 1; j < seqLen; j++) {
                unsigned char cc = (unsigned char)buf[i + j];
                if ((cc & 0xC0) != 0x80) {
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

static bool convertUtf8ToEscaped(const char* input, size_t inputLen, std::vector<char>& output)
{
    // Skip BOM if present
    const char* src = input;
    size_t srcLen = inputLen;
    if (srcLen >= 3 &&
        (unsigned char)src[0] == 0xEF &&
        (unsigned char)src[1] == 0xBB &&
        (unsigned char)src[2] == 0xBF) {
        src += 3;
        srcLen -= 3;
    }

    // Need null-terminated string for convertTextToWideText
    std::string nullTerminated(src, srcLen);

    wchar_t* wideText = nullptr;
    errno_t err = convertTextToWideText(nullTerminated.c_str(), &wideText);
    if (err != 0 || !wideText)
        return false;

    char* escapedText = nullptr;
    err = convertWideTextToEscapedText(wideText, &escapedText);
    free(wideText);

    if (err != 0 || !escapedText)
        return false;

    size_t escapedLen = strlen(escapedText);
    output.assign(escapedText, escapedText + escapedLen);

    free(escapedText);
    return true;
}

static HANDLE WINAPI hookedCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    HANDLE h = origCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
                               lpSecurityAttributes, dwCreationDisposition,
                               dwFlagsAndAttributes, hTemplateFile);

    if (h != INVALID_HANDLE_VALUE && isTextFile(lpFileName)) {
        // Only track files opened for reading
        if ((dwDesiredAccess & GENERIC_READ) && !(dwDesiredAccess & GENERIC_WRITE)) {
            EnterCriticalSection(&handleLock);
            trackedHandles[h] = {};
            LeaveCriticalSection(&handleLock);
        }
    }

    return h;
}

static BOOL WINAPI hookedReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped)
{
    EnterCriticalSection(&handleLock);
    auto it = trackedHandles.find(hFile);
    if (it == trackedHandles.end()) {
        LeaveCriticalSection(&handleLock);
        return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    }

    HandleState& state = it->second;

    // If we haven't read/converted the file yet, do it now
    if (state.pendingData.empty() && state.readOffset == 0) {
        LeaveCriticalSection(&handleLock);

        // Get file size
        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart == 0) {
            EnterCriticalSection(&handleLock);
            trackedHandles.erase(hFile);
            LeaveCriticalSection(&handleLock);
            return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
        }

        size_t totalSize = (size_t)fileSize.QuadPart;
        std::vector<char> fileContent(totalSize);

        // Read entire file
        DWORD totalRead = 0;
        DWORD bytesRead = 0;
        // Save current file position
        LARGE_INTEGER savedPos;
        LARGE_INTEGER zero = {};
        SetFilePointerEx(hFile, zero, &savedPos, FILE_CURRENT);
        // Seek to beginning
        SetFilePointerEx(hFile, zero, nullptr, FILE_BEGIN);

        while (totalRead < totalSize) {
            DWORD toRead = (DWORD)std::min(totalSize - totalRead, (size_t)0x7FFFFFFF);
            if (!origReadFile(hFile, fileContent.data() + totalRead, toRead, &bytesRead, nullptr) || bytesRead == 0) {
                break;
            }
            totalRead += bytesRead;
        }

        if (totalRead == 0) {
            // Restore file position
            SetFilePointerEx(hFile, savedPos, nullptr, FILE_BEGIN);
            EnterCriticalSection(&handleLock);
            trackedHandles.erase(hFile);
            LeaveCriticalSection(&handleLock);
            return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
        }

        fileContent.resize(totalRead);

        EnterCriticalSection(&handleLock);
        it = trackedHandles.find(hFile);
        if (it == trackedHandles.end()) {
            LeaveCriticalSection(&handleLock);
            SetFilePointerEx(hFile, savedPos, nullptr, FILE_BEGIN);
            return origReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
        }

        if (needsUtf8Conversion(fileContent.data(), fileContent.size())) {
            std::vector<char> converted;
            if (convertUtf8ToEscaped(fileContent.data(), fileContent.size(), converted)) {
                it->second.pendingData = std::move(converted);
                it->second.readOffset = 0;
            } else {
                // Conversion failed, pass through original data
                it->second.pendingData = std::move(fileContent);
                it->second.readOffset = 0;
            }
        } else {
            // No conversion needed, pass through original data
            it->second.pendingData = std::move(fileContent);
            it->second.readOffset = 0;
        }
    }

    // Serve data from pendingData
    HandleState& s = it != trackedHandles.end() ? it->second : trackedHandles[hFile];
    size_t remaining = s.pendingData.size() - s.readOffset;
    DWORD toCopy = (DWORD)std::min((size_t)nNumberOfBytesToRead, remaining);

    if (toCopy > 0) {
        memcpy(lpBuffer, s.pendingData.data() + s.readOffset, toCopy);
        s.readOffset += toCopy;
    }

    if (lpNumberOfBytesRead)
        *lpNumberOfBytesRead = toCopy;

    LeaveCriticalSection(&handleLock);
    return TRUE;
}

static BOOL WINAPI hookedCloseHandle(HANDLE hObject)
{
    EnterCriticalSection(&handleLock);
    trackedHandles.erase(hObject);
    LeaveCriticalSection(&handleLock);

    return origCloseHandle(hObject);
}

void Init(RunOptions options)
{
    if (!options.autoUtf8Conversion)
        return;

    InitializeCriticalSection(&handleLock);

    HMODULE exeModule = GetModuleHandle(nullptr);

    origCreateFileW = (decltype(origCreateFileW))IATHook::Hook(
        exeModule, "kernel32.dll", "CreateFileW", (void*)hookedCreateFileW);

    origReadFile = (decltype(origReadFile))IATHook::Hook(
        exeModule, "kernel32.dll", "ReadFile", (void*)hookedReadFile);

    origCloseHandle = (decltype(origCloseHandle))IATHook::Hook(
        exeModule, "kernel32.dll", "CloseHandle", (void*)hookedCloseHandle);
}

} // namespace FileRead
