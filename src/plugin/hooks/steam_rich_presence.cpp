#include "../iat_hook.h"
#include "../escape_tool.h"
#include <string>

namespace SteamRichPresence {

// Flat API function pointer type
typedef bool (__cdecl *SetRichPresenceFn)(void* self, const char* pchKey, const char* pchValue);

// Original function pointers
static SetRichPresenceFn origSetRichPresence = nullptr;

// For SteamAPI_Init fallback
typedef bool (__cdecl *SteamAPI_InitFn)();
typedef void* (__cdecl *SteamFriendsGetterFn)();
static SteamAPI_InitFn origSteamAPIInit = nullptr;
static void** vtableHookLocation = nullptr;
static SetRichPresenceFn origVtableSetRichPresence = nullptr;

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

// Hook for flat API: SteamAPI_ISteamFriends_SetRichPresence
static bool __cdecl hookedSetRichPresence(void* self, const char* pchKey, const char* pchValue) {
    if (pchValue && hasEscapedChars(pchValue)) {
        std::string utf8Value = convertEscapedToUtf8(pchValue);
        return origSetRichPresence(self, pchKey, utf8Value.c_str());
    }
    return origSetRichPresence(self, pchKey, pchValue);
}

// Vtable hook version of SetRichPresence (thiscall via ecx on x86, rcx on x64)
// On x64 Windows, thiscall == normal calling convention (first arg = this in rcx)
static bool vtableHookedSetRichPresence(void* self, const char* pchKey, const char* pchValue) {
    if (pchValue && hasEscapedChars(pchValue)) {
        std::string utf8Value = convertEscapedToUtf8(pchValue);
        return origVtableSetRichPresence(self, pchKey, utf8Value.c_str());
    }
    return origVtableSetRichPresence(self, pchKey, pchValue);
}

// Hook the vtable of ISteamFriends to intercept SetRichPresence
static void hookVtable(void* steamFriends) {
    if (!steamFriends) return;

    // ISteamFriends vtable: SetRichPresence is at index 43 in ISteamFriends017
    void** vtable = *reinterpret_cast<void***>(steamFriends);
    constexpr int kSetRichPresenceIndex = 43;

    origVtableSetRichPresence = reinterpret_cast<SetRichPresenceFn>(vtable[kSetRichPresenceIndex]);
    vtableHookLocation = &vtable[kSetRichPresenceIndex];

    DWORD oldProtect;
    VirtualProtect(vtableHookLocation, sizeof(void*), PAGE_READWRITE, &oldProtect);
    *vtableHookLocation = reinterpret_cast<void*>(vtableHookedSetRichPresence);
    VirtualProtect(vtableHookLocation, sizeof(void*), oldProtect, &oldProtect);
}

// Hooked SteamAPI_Init: call original, then hook vtable
static bool __cdecl hookedSteamAPIInit() {
    bool result = origSteamAPIInit();
    if (!result) return false;

    // Get ISteamFriends* via SteamAPI_SteamFriends_v017
    HMODULE steamApi = GetModuleHandleA("steam_api64.dll");
    if (!steamApi)
        steamApi = GetModuleHandleA("steam_api.dll");
    if (!steamApi) return true;

    auto getSteamFriends = reinterpret_cast<SteamFriendsGetterFn>(
        GetProcAddress(steamApi, "SteamAPI_SteamFriends_v017"));
    if (!getSteamFriends) return true;

    void* steamFriends = getSteamFriends();
    if (steamFriends) {
        hookVtable(steamFriends);
    }

    return true;
}

void Init() {
    HMODULE exeModule = GetModuleHandle(nullptr);

    // Primary: try to IAT hook the flat API function
    origSetRichPresence = reinterpret_cast<SetRichPresenceFn>(
        IATHook::Hook(exeModule, "steam_api64.dll",
                      "SteamAPI_ISteamFriends_SetRichPresence",
                      reinterpret_cast<void*>(hookedSetRichPresence)));

    if (origSetRichPresence)
        return; // IAT hook succeeded

    // Fallback: hook SteamAPI_Init to later do vtable hook
    origSteamAPIInit = reinterpret_cast<SteamAPI_InitFn>(
        IATHook::Hook(exeModule, "steam_api64.dll",
                      "SteamAPI_Init",
                      reinterpret_cast<void*>(hookedSteamAPIInit)));
}

} // namespace SteamRichPresence
