// Steam Rich Presence UTF-8 conversion hook
//
// EU4 uses an internal escape encoding (bytes 0x10-0x13 as markers) for CJK text.
// When the game calls ISteamFriends::SetRichPresence(), the value contains these
// escape markers instead of proper UTF-8, causing garbled text in Steam's UI.
//
// This hook intercepts SetRichPresence via the ISteamFriends vtable, converts the
// escape-encoded text to UTF-8 on the fly, then forwards to the original function.
//
// The vtable index for SetRichPresence is discovered dynamically by parsing the
// machine code of the flat API export SteamAPI_ISteamFriends_SetRichPresence:
//   - Wine/Proton + original Steam SDK: the export is a vtable dispatch stub
//     (mov rax,[rcx]; jmp [rax+offset]) — the offset directly gives the index.
//   - Crack DLLs: the export is a thunk that forwards to the original DLL's stub.
//     We follow the thunk and parse the target as a vtable dispatch.

#include "../escape_tool.h"
#include <csetjmp>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace SteamRichPresence {

    // ---- Forward declarations ----

    static void logOpen();
    static void logClose();
    static void logMsg(const char* fmt, ...);
    static void logBytes(const char* label, const void* data, int len);

    static bool        hasEscapedChars(const char* text);
    static std::string convertToUtf8(const char* text);

    typedef bool (*SetRichPresenceFn)(void* self, const char* pchKey, const char* pchValue);
    static SetRichPresenceFn origSetRichPresence = nullptr;
    static bool hookedSetRichPresence(void* self, const char* pchKey, const char* pchValue);

    static int   parseVtableDispatch(const uint8_t* fn);
    static void* resolveThunkTarget(const uint8_t* fn);

    static void* resolveSteamFriends(HMODULE steamApi);
    static int   findVtableIndex(HMODULE steamApi, void* steamFriends);
    static bool  hookVtable(void* steamFriends, int index);

    // ---- Init (entry point) ----

    // Thread-local jump buffer for safe recovery from exceptions
    static thread_local jmp_buf initJmpBuf;
    static volatile DWORD       initThreadId = 0;

    void Init() {
        // Only proceed if steam_api64.dll exists (loaded or on disk)
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

        // Install a VEH that catches fatal crashes in our init thread and
        // safely jumps back via longjmp instead of ExitThread.
        auto veh = AddVectoredExceptionHandler(1, [](PEXCEPTION_POINTERS ep) -> LONG {
            if (GetCurrentThreadId() != initThreadId)
                return EXCEPTION_CONTINUE_SEARCH;

            DWORD code = ep->ExceptionRecord->ExceptionCode;

            // Only intercept fatal exceptions (access violation, illegal instruction, etc.)
            if (code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_ILLEGAL_INSTRUCTION &&
                code != EXCEPTION_STACK_OVERFLOW && code != EXCEPTION_INT_DIVIDE_BY_ZERO &&
                code != EXCEPTION_PRIV_INSTRUCTION && code != EXCEPTION_IN_PAGE_ERROR)
                return EXCEPTION_CONTINUE_SEARCH;

            logMsg("Exception 0x%08lX at %p, aborting hook\n", code,
                   ep->ExceptionRecord->ExceptionAddress);
            logClose();
            longjmp(initJmpBuf, 1);
            return EXCEPTION_CONTINUE_SEARCH; // unreachable
        });

        HANDLE hThread = CreateThread(
            nullptr, 0,
            [](LPVOID vehHandle) -> DWORD {
                initThreadId = GetCurrentThreadId();

                logOpen();
                logMsg("=== Steam Rich Presence hook init ===\n");

                // Set up safe recovery point — if any exception occurs,
                // longjmp returns here with value 1
                if (setjmp(initJmpBuf) != 0) {
                    // Exception occurred, clean up and exit gracefully
                    RemoveVectoredExceptionHandler(vehHandle);
                    initThreadId = 0;
                    return 0;
                }

                // Wait for steam_api64.dll to load (up to 30s)
                HMODULE api = nullptr;
                for (int i = 0; i < 300 && !api; i++) {
                    Sleep(100);
                    api = GetModuleHandleA("steam_api64.dll");
                }
                if (!api) {
                    logMsg("steam_api64.dll not found\n");
                    logClose();
                    RemoveVectoredExceptionHandler(vehHandle);
                    initThreadId = 0;
                    return 0;
                }
                logMsg("steam_api64.dll at %p\n", api);

                // Wait for Steam interfaces to initialize
                logMsg("Sleeping 3s...\n");
                Sleep(3000);
                logMsg("Sleep done, resolving ISteamFriends...\n");

                // Resolve ISteamFriends interface
                void* friends = resolveSteamFriends(api);
                if (!friends) {
                    logMsg("Failed to resolve ISteamFriends\n");
                    logClose();
                    RemoveVectoredExceptionHandler(vehHandle);
                    initThreadId = 0;
                    return 0;
                }

                // Discover vtable index and install hook
                logMsg("Calling findVtableIndex...\n");
                int index = findVtableIndex(api, friends);
                logMsg("findVtableIndex returned %d\n", index);
                if (index >= 0) {
                    logMsg("Calling hookVtable...\n");
                    if (!hookVtable(friends, index))
                        logMsg("hookVtable failed\n");
                } else {
                    logMsg("Hook skipped (bad index)\n");
                }

                logMsg("=== Init complete ===\n");
                logClose();
                RemoveVectoredExceptionHandler(vehHandle);
                initThreadId = 0;
                return 0;
            },
            reinterpret_cast<LPVOID>(veh), 0, nullptr);
        if (hThread)
            CloseHandle(hThread);
    }

    // ---- Vtable hook setup ----

    static void* resolveSteamFriends(HMODULE steamApi) {
        typedef void* (*GetterFn)();
        const char* versions[] = {
            "SteamAPI_SteamFriends_v017",
            "SteamAPI_SteamFriends_v016",
            "SteamAPI_SteamFriends_v015",
        };
        for (auto ver : versions) {
            auto fn = reinterpret_cast<GetterFn>(GetProcAddress(steamApi, ver));
            if (!fn)
                continue;
            logMsg("  Calling %s at %p...\n", ver, fn);
            void* iface = fn();
            logMsg("  %s() -> %p\n", ver, iface);
            if (iface)
                return iface;
        }
        return nullptr;
    }

    // Discover the vtable index by examining the flat API function's machine code.
    // Tries vtable dispatch first, then follows thunks (crack DLL -> original DLL).
    static int findVtableIndex(HMODULE steamApi, void* steamFriends) {
        logMsg("findVtableIndex: GetProcAddress(SetRichPresence)...\n");
        auto fn = reinterpret_cast<const uint8_t*>(
            GetProcAddress(steamApi, "SteamAPI_ISteamFriends_SetRichPresence"));
        if (!fn) {
            logMsg("findVtableIndex: export not found\n");
            return -1;
        }
        logMsg("findVtableIndex: fn at %p\n", fn);

        logBytes("Flat API bytes", fn, 16);

        // Direct vtable dispatch? (Wine/Proton, original Steam SDK)
        logMsg("findVtableIndex: trying parseVtableDispatch (direct)...\n");
        int index = parseVtableDispatch(fn);
        if (index >= 0) {
            logMsg("findVtableIndex: direct dispatch -> index=%d\n", index);
            return index;
        }

        // Thunk? Follow it and try again (crack DLL -> original DLL's dispatch stub)
        logMsg("findVtableIndex: trying resolveThunkTarget...\n");
        void* target = resolveThunkTarget(fn);
        if (!target) {
            logMsg("Not a recognized pattern\n");
            return -1;
        }

        logMsg("Thunk -> %p\n", target);
        logBytes("Target bytes", target, 16);

        logMsg("findVtableIndex: trying parseVtableDispatch (thunk target)...\n");
        index = parseVtableDispatch(reinterpret_cast<const uint8_t*>(target));
        if (index >= 0) {
            logMsg("findVtableIndex: thunk dispatch -> index=%d\n", index);
            return index;
        }

        // Last resort: scan vtable for direct pointer match
        logMsg("findVtableIndex: scanning vtable for match...\n");
        void** vtable = *reinterpret_cast<void***>(steamFriends);
        logMsg("findVtableIndex: vtable at %p\n", vtable);
        if (vtable) {
            for (int i = 0; i < 200; i++) {
                if (vtable[i] == target) {
                    logMsg("vtable[%d] matches target\n", i);
                    return i;
                }
            }
        }

        logMsg("Failed to determine vtable index\n");
        return -1;
    }

    static bool hookVtable(void* steamFriends, int index) {
        logMsg("hookVtable: index=%d, reading vtable ptr...\n", index);
        void** vtable = *reinterpret_cast<void***>(steamFriends);
        if (!vtable) {
            logMsg("hookVtable: vtable is null\n");
            return false;
        }

        origSetRichPresence = reinterpret_cast<SetRichPresenceFn>(vtable[index]);
        logMsg("hookVtable: orig=%p\n", origSetRichPresence);
        if (!origSetRichPresence)
            return false;

        DWORD oldProtect;
        logMsg("hookVtable: VirtualProtect...\n");
        if (!VirtualProtect(&vtable[index], sizeof(void*), PAGE_READWRITE, &oldProtect)) {
            logMsg("hookVtable: VirtualProtect failed (err=%lu)\n", GetLastError());
            return false;
        }
        vtable[index] = reinterpret_cast<void*>(hookedSetRichPresence);
        VirtualProtect(&vtable[index], sizeof(void*), oldProtect, &oldProtect);

        logMsg("Hooked vtable[%d]: %p -> %p\n", index, origSetRichPresence, hookedSetRichPresence);
        return true;
    }

    // ---- x86-64 instruction parsing ----

    // Parse a vtable dispatch stub: mov rax,[rcx]; jmp/call [rax+offset]
    // Returns vtable index (offset/8), or -1 if the bytes don't match.
    //
    //   48 8B 01                 mov rax, [rcx]
    //   [48] FF 60 XX            jmp [rax+disp8]
    //   [48] FF A0 XX XX XX XX   jmp [rax+disp32]
    static int parseVtableDispatch(const uint8_t* fn) {
        if (fn[0] != 0x48 || fn[1] != 0x8B || fn[2] != 0x01) {
            logMsg("  parseVtableDispatch: no match (header %02X %02X %02X)\n", fn[0], fn[1], fn[2]);
            return -1;
        }

        const uint8_t* p = fn + 3;
        if (*p >= 0x40 && *p <= 0x4F)
            p++;

        if (*p != 0xFF)
            return -1;

        uint8_t modrm = p[1];
        uint8_t mod   = (modrm >> 6) & 3;
        uint8_t reg   = (modrm >> 3) & 7;
        uint8_t rm    = modrm & 7;

        if ((reg != 4 && reg != 2) || rm != 0 || mod == 0 || mod == 3) {
            logMsg("  parseVtableDispatch: modrm mismatch (mod=%d reg=%d rm=%d)\n", mod, reg, rm);
            return -1;
        }

        uint32_t offset = 0;
        if (mod == 1)
            offset = p[2];
        else
            memcpy(&offset, &p[2], 4);

        if (offset == 0 || offset % 8 != 0 || offset / 8 >= 200) {
            logMsg("  parseVtableDispatch: bad offset 0x%X\n", offset);
            return -1;
        }

        logMsg("  Vtable dispatch: offset=0x%X, index=%u\n", offset, offset / 8);
        return static_cast<int>(offset / 8);
    }

    // Resolve the target address of a thunk (indirect jump via RIP-relative pointer).
    //
    //   48 8B 05 [d32] {FF E0 | 48 FF E0}   mov rax,[rip+d]; jmp rax
    //   [48] FF 25 [d32]                     jmp [rip+d]
    static void* resolveThunkTarget(const uint8_t* fn) {
        if (fn[0] == 0x48 && fn[1] == 0x8B && fn[2] == 0x05) {
            if ((fn[7] == 0xFF && fn[8] == 0xE0) ||
                (fn[7] == 0x48 && fn[8] == 0xFF && fn[9] == 0xE0)) {
                int32_t disp;
                memcpy(&disp, &fn[3], 4);
                auto ptr = const_cast<uint8_t*>(fn) + 7 + disp;
                logMsg("  resolveThunk: mov+jmp pattern, disp=%d, deref %p\n", disp, ptr);
                return *reinterpret_cast<void**>(ptr);
            }
        }

        int off = (fn[0] == 0x48) ? 1 : 0;
        if (fn[off] == 0xFF && fn[off + 1] == 0x25) {
            int32_t disp;
            memcpy(&disp, &fn[off + 2], 4);
            auto ptr = const_cast<uint8_t*>(fn) + off + 6 + disp;
            logMsg("  resolveThunk: jmp [rip+d] pattern, disp=%d, deref %p\n", disp, ptr);
            return *reinterpret_cast<void**>(ptr);
        }

        logMsg("  resolveThunk: no pattern matched\n");
        return nullptr;
    }

    // ---- Hook callback (called at runtime for each SetRichPresence) ----

    static bool hookedSetRichPresence(void* self, const char* pchKey, const char* pchValue) {
        if (pchValue && hasEscapedChars(pchValue)) {
            std::string utf8 = convertToUtf8(pchValue);
            logMsg("Convert: key=\"%s\" -> \"%s\"\n", pchKey ? pchKey : "(null)", utf8.c_str());
            return origSetRichPresence(self, pchKey, utf8.c_str());
        }
        return origSetRichPresence(self, pchKey, pchValue);
    }

    // ---- Text conversion ----

    static bool hasEscapedChars(const char* text) {
        if (!text)
            return false;
        for (auto p = reinterpret_cast<const uint8_t*>(text); *p; ++p)
            if (*p >= 0x10 && *p <= 0x13)
                return true;
        return false;
    }

    // The game treats each byte of escape-encoded text as a character and re-encodes
    // bytes >= 0x80 as UTF-8 before passing to SetRichPresence (e.g. 0xAF -> C2 AF).
    // We collapse these spurious 2-byte sequences back to single bytes, then decode
    // the escape encoding to produce proper UTF-8.
    static std::string convertToUtf8(const char* text) {
        std::string raw;
        for (auto p = reinterpret_cast<const uint8_t*>(text); *p;) {
            if (p[0] >= 0xC2 && p[0] <= 0xC3 && (p[1] & 0xC0) == 0x80) {
                raw += static_cast<char>(((p[0] & 0x1F) << 6) | (p[1] & 0x3F));
                p += 2;
            } else {
                raw += static_cast<char>(*p++);
            }
        }
        return convertWideTextToUtf8(convertEscapedTextToWideText(raw));
    }

    // ---- Logging ----

    static FILE* logFile = nullptr;

    static void logOpen() {
        if (logFile)
            return;
        char path[MAX_PATH];
        if (GetModuleFileNameA(nullptr, path, MAX_PATH)) {
            char* s = strrchr(path, '\\');
            if (s)
                strcpy(s + 1, "steam_rp_debug.log");
            logFile = fopen(path, "w");
        }
    }

    static void logClose() {
        if (logFile) {
            fclose(logFile);
            logFile = nullptr;
        }
    }

    static void logMsg(const char* fmt, ...) {
        if (!logFile)
            return;
        va_list ap;
        va_start(ap, fmt);
        vfprintf(logFile, fmt, ap);
        va_end(ap);
        fflush(logFile);
    }

    static void logBytes(const char* label, const void* data, int len) {
        if (!logFile)
            return;
        fprintf(logFile, "%s: ", label);
        for (int i = 0; i < len; i++)
            fprintf(logFile, "%02X ", reinterpret_cast<const uint8_t*>(data)[i]);
        fprintf(logFile, "\n");
        fflush(logFile);
    }

} // namespace SteamRichPresence
