#include "../escape_tool.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace SteamRichPresence {

typedef bool (*SetRichPresenceFn)(void* self, const char* pchKey, const char* pchValue);
typedef void* (*SteamInterfaceGetterFn)();

typedef uint64_t (*FlatGetSteamIDFn)(void* self);
typedef int (*FlatGetKeyCountFn)(void* self, uint64_t steamID);
typedef const char* (*FlatGetKeyByIndexFn)(void* self, uint64_t steamID, int iKey);
typedef const char* (*FlatGetRichPresenceFn)(void* self, uint64_t steamID, const char* pchKey);
typedef bool (*FlatSetRichPresenceFn)(void* self, const char* pchKey, const char* pchValue);

static SetRichPresenceFn origSetRichPresence = nullptr;

static FILE* logFile = nullptr;

static void logOpen() {
    if (logFile) return;
    char path[MAX_PATH];
    if (GetModuleFileNameA(nullptr, path, MAX_PATH)) {
        char* s = strrchr(path, '\\');
        if (s) strcpy(s + 1, "steam_rp_debug.log");
        logFile = fopen(path, "w");
    }
}

static void logMsg(const char* fmt, ...) {
    if (!logFile) return;
    va_list ap;
    va_start(ap, fmt);
    vfprintf(logFile, fmt, ap);
    va_end(ap);
    fflush(logFile);
}

static void logHex(const char* label, const char* data) {
    if (!logFile || !data) return;
    fprintf(logFile, "%s: \"", label);
    for (const uint8_t* p = reinterpret_cast<const uint8_t*>(data); *p; ++p)
        fprintf(logFile, "\\x%02X", *p);
    fprintf(logFile, "\"\n");
    fflush(logFile);
}

// Check for escape encoding markers (0x10-0x13)
static bool hasEscapedChars(const char* text) {
    if (!text) return false;
    for (const uint8_t* p = reinterpret_cast<const uint8_t*>(text); *p; ++p) {
        if (*p >= 0x10 && *p <= 0x13)
            return true;
    }
    return false;
}

// The game re-encodes raw bytes >= 0x80 as UTF-8 before SetRichPresence
// (e.g. 0xAF -> C2 AF, 0xFC -> C3 BC). Collapse them back to single bytes.
static std::string collapseUtf8ToRawBytes(const char* text) {
    std::string result;
    const uint8_t* p = reinterpret_cast<const uint8_t*>(text);
    while (*p) {
        if (p[0] >= 0xC2 && p[0] <= 0xC3 && (p[1] & 0xC0) == 0x80) {
            // Two-byte UTF-8 for U+0080-U+00FF -> single byte
            result += static_cast<char>(((p[0] & 0x1F) << 6) | (p[1] & 0x3F));
            p += 2;
        } else {
            result += static_cast<char>(*p++);
        }
    }
    return result;
}

// Collapse spurious UTF-8 -> decode escape encoding -> encode as proper UTF-8
static std::string convertToUtf8(const char* text) {
    std::string rawBytes = collapseUtf8ToRawBytes(text);
    std::wstring wide = convertEscapedTextToWideText(rawBytes);
    return convertWideTextToUtf8(wide);
}

// Scan SteamAPI_ISteamFriends_SetRichPresence to find the vtable offset.
// The flat API wrapper does: mov rax,[rcx]; jmp [rax+offset]
static int detectVtableIndex(HMODULE steamApi) {
    auto fn = reinterpret_cast<const uint8_t*>(
        GetProcAddress(steamApi, "SteamAPI_ISteamFriends_SetRichPresence"));
    if (!fn) return -1;

    logMsg("Flat API function at %p, scanning bytes:\n  ", fn);
    for (int i = 0; i < 32; i++) logMsg("%02X ", fn[i]);
    logMsg("\n");

    for (int i = 0; i < 28; i++) {
        int ffPos = -1;
        if (fn[i] == 0xFF)
            ffPos = i;
        else if (fn[i] >= 0x40 && fn[i] <= 0x4F && fn[i + 1] == 0xFF)
            ffPos = i + 1; // REX prefix
        if (ffPos < 0) continue;

        uint8_t modrm = fn[ffPos + 1];
        uint8_t mod = (modrm >> 6) & 3;
        uint8_t reg = (modrm >> 3) & 7;
        uint8_t rm = modrm & 7;

        if (reg != 2 && reg != 4) { i = ffPos; continue; } // not call/jmp
        if (rm == 4 || (mod == 0 && rm == 5)) { i = ffPos; continue; } // SIB/RIP-relative

        if (mod == 1) {
            uint32_t disp = fn[ffPos + 2]; // [reg + imm8]
            if (disp % 8 == 0 && disp / 8 > 0 && disp / 8 < 200) {
                logMsg("Detected vtable index %u (offset 0x%X) via imm8 at byte %d\n", disp / 8, disp, ffPos);
                return disp / 8;
            }
        } else if (mod == 2) {
            uint32_t disp;
            memcpy(&disp, &fn[ffPos + 2], 4); // [reg + imm32]
            if (disp % 8 == 0 && disp / 8 > 0 && disp / 8 < 200) {
                logMsg("Detected vtable index %u (offset 0x%X) via imm32 at byte %d\n", disp / 8, disp, ffPos);
                return disp / 8;
            }
        }

        i = ffPos;
    }

    logMsg("Failed to detect vtable index from flat API\n");
    return -1;
}

// Vtable hook: intercept SetRichPresence and convert escaped text to UTF-8
static bool hookedSetRichPresence(void* self, const char* pchKey, const char* pchValue) {
    if (pchValue && hasEscapedChars(pchValue)) {
        logHex(pchKey ? pchKey : "(null)", pchValue);
        std::string utf8Value = convertToUtf8(pchValue);
        logMsg("  -> UTF-8: \"%s\"\n", utf8Value.c_str());
        return origSetRichPresence(self, pchKey, utf8Value.c_str());
    }
    return origSetRichPresence(self, pchKey, pchValue);
}

static bool hookVtable(void* steamFriends, int vtableIndex) {
    if (!steamFriends || vtableIndex < 0) return false;

    void** vtable = *reinterpret_cast<void***>(steamFriends);
    if (!vtable) return false;

    origSetRichPresence = reinterpret_cast<SetRichPresenceFn>(vtable[vtableIndex]);
    if (!origSetRichPresence) return false;

    logMsg("Hooking vtable[%d] = %p -> %p\n", vtableIndex, origSetRichPresence, hookedSetRichPresence);

    DWORD oldProtect;
    if (!VirtualProtect(&vtable[vtableIndex], sizeof(void*), PAGE_READWRITE, &oldProtect))
        return false;

    vtable[vtableIndex] = reinterpret_cast<void*>(hookedSetRichPresence);
    VirtualProtect(&vtable[vtableIndex], sizeof(void*), oldProtect, &oldProtect);

    return true;
}

// Polling fallback: periodically fix garbled values via flat API

struct FlatAPI {
    FlatSetRichPresenceFn setRP;
    FlatGetSteamIDFn getSteamID;
    FlatGetKeyCountFn getKeyCount;
    FlatGetKeyByIndexFn getKeyByIndex;
    FlatGetRichPresenceFn getRichPresence;
    void* steamFriends;
    void* steamUser;
};

static bool initFlatAPI(HMODULE steamApi, void* steamFriends, FlatAPI& api) {
    api.steamFriends = steamFriends;

    api.setRP = reinterpret_cast<FlatSetRichPresenceFn>(
        GetProcAddress(steamApi, "SteamAPI_ISteamFriends_SetRichPresence"));
    api.getSteamID = reinterpret_cast<FlatGetSteamIDFn>(
        GetProcAddress(steamApi, "SteamAPI_ISteamUser_GetSteamID"));
    api.getKeyCount = reinterpret_cast<FlatGetKeyCountFn>(
        GetProcAddress(steamApi, "SteamAPI_ISteamFriends_GetFriendRichPresenceKeyCount"));
    api.getKeyByIndex = reinterpret_cast<FlatGetKeyByIndexFn>(
        GetProcAddress(steamApi, "SteamAPI_ISteamFriends_GetFriendRichPresenceKeyByIndex"));
    api.getRichPresence = reinterpret_cast<FlatGetRichPresenceFn>(
        GetProcAddress(steamApi, "SteamAPI_ISteamFriends_GetFriendRichPresence"));

    if (!api.setRP || !api.getSteamID || !api.getKeyCount || !api.getKeyByIndex || !api.getRichPresence) {
        logMsg("initFlatAPI: missing functions (setRP=%p getSteamID=%p getKeyCount=%p getKeyByIndex=%p getRichPresence=%p)\n",
               api.setRP, api.getSteamID, api.getKeyCount, api.getKeyByIndex, api.getRichPresence);
        return false;
    }

    const char* userVersions[] = {
        "SteamAPI_SteamUser_v023", "SteamAPI_SteamUser_v022", "SteamAPI_SteamUser_v021",
    };
    SteamInterfaceGetterFn userGetter = nullptr;
    for (auto ver : userVersions) {
        userGetter = reinterpret_cast<SteamInterfaceGetterFn>(GetProcAddress(steamApi, ver));
        if (userGetter) break;
    }
    if (!userGetter) { logMsg("initFlatAPI: no SteamUser getter\n"); return false; }

    api.steamUser = userGetter();
    if (!api.steamUser) { logMsg("initFlatAPI: SteamUser is null\n"); return false; }

    return true;
}

static void fixValuesViaFlatAPI(const FlatAPI& api) {
    uint64_t localID = api.getSteamID(api.steamUser);
    int count = api.getKeyCount(api.steamFriends, localID);

    struct KV { std::string key, val; };
    std::vector<KV> toFix;

    for (int i = 0; i < count; i++) {
        const char* key = api.getKeyByIndex(api.steamFriends, localID, i);
        if (!key) continue;
        const char* value = api.getRichPresence(api.steamFriends, localID, key);
        if (!value || !hasEscapedChars(value)) continue;

        toFix.push_back({key, convertToUtf8(value)});
    }

    for (const auto& kv : toFix) {
        logMsg("Poll fix: \"%s\" -> \"%s\"\n", kv.key.c_str(), kv.val.c_str());
        api.setRP(api.steamFriends, kv.key.c_str(), kv.val.c_str());
    }
}

// Background thread: wait for Steam API, install hook, then poll as fallback
static DWORD WINAPI DelayedInit(LPVOID) {
    logOpen();
    logMsg("=== Steam Rich Presence hook init ===\n");

    // Wait for steam_api64.dll (up to 30s)
    HMODULE steamApi = nullptr;
    for (int i = 0; i < 300 && !steamApi; i++) {
        Sleep(100);
        steamApi = GetModuleHandleA("steam_api64.dll");
    }
    if (!steamApi) { logMsg("steam_api64.dll not found\n"); return 0; }
    logMsg("steam_api64.dll at %p\n", steamApi);

    // Auto-detect vtable index from flat API wrapper
    int vtableIndex = detectVtableIndex(steamApi);
    if (vtableIndex < 0) {
        logMsg("Using fallback vtable index 43\n");
        vtableIndex = 43;
    }

    // Resolve ISteamFriends getter
    const char* versions[] = {
        "SteamAPI_SteamFriends_v017", "SteamAPI_SteamFriends_v016", "SteamAPI_SteamFriends_v015",
    };
    SteamInterfaceGetterFn getter = nullptr;
    for (auto ver : versions) {
        getter = reinterpret_cast<SteamInterfaceGetterFn>(GetProcAddress(steamApi, ver));
        if (getter) { logMsg("Using %s\n", ver); break; }
    }
    if (!getter) { logMsg("No SteamFriends getter found\n"); return 0; }

    // Wait for ISteamFriends (up to 10s)
    void* steamFriends = nullptr;
    for (int i = 0; i < 100 && !steamFriends; i++) {
        Sleep(100);
        steamFriends = getter();
    }
    if (!steamFriends) { logMsg("SteamFriends is null (Steam not running?)\n"); return 0; }
    logMsg("SteamFriends at %p\n", steamFriends);

    // Vtable hook for real-time interception
    bool vtableHooked = hookVtable(steamFriends, vtableIndex);
    if (vtableHooked)
        logMsg("Vtable hook installed successfully\n");
    else
        logMsg("Vtable hook failed, relying on polling fallback\n");

    // Polling fallback via flat API
    FlatAPI flatApi = {};
    bool hasFlatApi = initFlatAPI(steamApi, steamFriends, flatApi);

    if (hasFlatApi) {
        logMsg("Running initial fix...\n");
        fixValuesViaFlatAPI(flatApi);

        logMsg("Starting polling loop (every 5 seconds)\n");
        for (;;) {
            Sleep(5000);
            fixValuesViaFlatAPI(flatApi);
        }
    } else {
        logMsg("Flat API init failed, no polling fallback available\n");
    }

    logMsg("=== Init complete ===\n");
    return 0;
}

void Init() {
    // Skip if steam_api64.dll is neither loaded nor present on disk
    HMODULE steamApi = GetModuleHandleA("steam_api64.dll");
    if (!steamApi) {
        char path[MAX_PATH];
        if (GetModuleFileNameA(nullptr, path, MAX_PATH)) {
            char* s = strrchr(path, '\\');
            if (s) {
                strcpy(s + 1, "steam_api64.dll");
                if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES)
                    return;
            }
        }
    }

    HANDLE hThread = CreateThread(nullptr, 0, DelayedInit, nullptr, 0, nullptr);
    if (hThread)
        CloseHandle(hThread);
}

} // namespace SteamRichPresence
