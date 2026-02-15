#include "../escape_tool.h"
#include <string>

namespace SteamRichPresence {

// Function pointer type matching ISteamFriends::SetRichPresence on x64
// x64 Windows has only one calling convention: this in rcx, args in rdx/r8/r9
typedef bool (*SetRichPresenceFn)(void* self, const char* pchKey, const char* pchValue);
typedef void* (*SteamFriendsGetterFn)();

// Original function pointer saved from vtable
static SetRichPresenceFn origSetRichPresence = nullptr;

// SetRichPresence vtable index in ISteamFriends (v015/v016/v017 all use index 43)
constexpr int kSetRichPresenceIndex = 43;

// Check if text contains escape markers (0x10-0x13)
static bool hasEscapedChars(const char* text) {
    if (!text) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
        if (*p >= 0x10 && *p <= 0x13)
            return true;
    }
    return false;
}

// Convert escaped text back to UTF-8
static std::string convertEscapedToUtf8(const char* text) {
    std::wstring wide = convertEscapedTextToWideText(std::string(text));
    return convertWideTextToUtf8(wide);
}

// Hook function: convert escaped text to UTF-8 before calling original
static bool hookedSetRichPresence(void* self, const char* pchKey, const char* pchValue) {
    if (pchValue && hasEscapedChars(pchValue)) {
        std::string utf8Value = convertEscapedToUtf8(pchValue);
        return origSetRichPresence(self, pchKey, utf8Value.c_str());
    }
    return origSetRichPresence(self, pchKey, pchValue);
}

// Hook the vtable of ISteamFriends to intercept SetRichPresence
static bool hookVtable(void* steamFriends) {
    if (!steamFriends) return false;

    void** vtable = *reinterpret_cast<void***>(steamFriends);
    if (!vtable) return false;

    origSetRichPresence = reinterpret_cast<SetRichPresenceFn>(vtable[kSetRichPresenceIndex]);
    if (!origSetRichPresence) return false;

    DWORD oldProtect;
    if (!VirtualProtect(&vtable[kSetRichPresenceIndex], sizeof(void*), PAGE_READWRITE, &oldProtect))
        return false;

    vtable[kSetRichPresenceIndex] = reinterpret_cast<void*>(hookedSetRichPresence);
    VirtualProtect(&vtable[kSetRichPresenceIndex], sizeof(void*), oldProtect, &oldProtect);

    return true;
}

// Background thread: wait for Steam API to initialize, then hook vtable
static DWORD WINAPI DelayedInit(LPVOID) {
    // Wait for steam_api64.dll to be loaded (up to 60 seconds)
    HMODULE steamApi = nullptr;
    for (int i = 0; i < 600 && !steamApi; i++) {
        Sleep(100);
        steamApi = GetModuleHandleA("steam_api64.dll");
    }
    if (!steamApi)
        return 0;

    // Find ISteamFriends accessor (try multiple SDK versions)
    const char* versions[] = {
        "SteamAPI_SteamFriends_v017",
        "SteamAPI_SteamFriends_v016",
        "SteamAPI_SteamFriends_v015",
    };

    SteamFriendsGetterFn getter = nullptr;
    for (auto ver : versions) {
        getter = reinterpret_cast<SteamFriendsGetterFn>(GetProcAddress(steamApi, ver));
        if (getter) break;
    }
    if (!getter)
        return 0;

    // Wait for SteamFriends interface to be available (up to 60 seconds)
    // It becomes non-null after SteamAPI_Init completes
    void* steamFriends = nullptr;
    for (int i = 0; i < 600 && !steamFriends; i++) {
        Sleep(100);
        steamFriends = getter();
    }
    if (!steamFriends)
        return 0;

    hookVtable(steamFriends);
    return 0;
}

void Init() {
    // Launch background thread for delayed initialization
    // Cannot hook immediately because steam_api64.dll may not be loaded yet
    HANDLE hThread = CreateThread(nullptr, 0, DelayedInit, nullptr, 0, nullptr);
    if (hThread)
        CloseHandle(hThread);
}

} // namespace SteamRichPresence
