#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <cstring>

namespace IATHook {

// Patch the IAT of the given module to replace a function imported from target_dll.
// Returns the original function pointer, or nullptr on failure.
inline void* Hook(HMODULE module, const char* target_dll, const char* func_name, void* hook_func)
{
    if (!module || !target_dll || !func_name || !hook_func)
        return nullptr;

    auto dos = (PIMAGE_DOS_HEADER)module;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
        return nullptr;

    auto nt = (PIMAGE_NT_HEADERS)((uintptr_t)module + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE)
        return nullptr;

    auto importDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (importDir.Size == 0)
        return nullptr;

    auto importDesc = (PIMAGE_IMPORT_DESCRIPTOR)((uintptr_t)module + importDir.VirtualAddress);

    for (; importDesc->Name != 0; importDesc++) {
        auto dllName = (const char*)((uintptr_t)module + importDesc->Name);

        if (_stricmp(dllName, target_dll) != 0)
            continue;

        auto origThunk = (PIMAGE_THUNK_DATA)((uintptr_t)module + importDesc->OriginalFirstThunk);
        auto iatThunk = (PIMAGE_THUNK_DATA)((uintptr_t)module + importDesc->FirstThunk);

        for (; origThunk->u1.AddressOfData != 0; origThunk++, iatThunk++) {
            if (IMAGE_SNAP_BY_ORDINAL(origThunk->u1.Ordinal))
                continue;

            auto importByName = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)module + origThunk->u1.AddressOfData);

            if (strcmp(importByName->Name, func_name) != 0)
                continue;

            void* original = (void*)iatThunk->u1.Function;

            DWORD oldProtect;
            VirtualProtect(&iatThunk->u1.Function, sizeof(uintptr_t), PAGE_READWRITE, &oldProtect);
            iatThunk->u1.Function = (uintptr_t)hook_func;
            VirtualProtect(&iatThunk->u1.Function, sizeof(uintptr_t), oldProtect, &oldProtect);

            return original;
        }
    }

    return nullptr;
}

} // namespace IATHook
