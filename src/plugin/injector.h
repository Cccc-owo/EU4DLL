/*
 *  Injectors - Base Header
 *
 *  Copyright (C) 2012-2014 LINK/2012 <dma_2012@hotmail.com>
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty. In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <windows.h>

namespace Injector {
    inline bool ProtectMemory(void* addr, size_t size, DWORD protection) {
        return VirtualProtect(addr, size, protection, &protection) != 0;
    }

    inline bool UnprotectMemory(void* addr, size_t size, DWORD& out_oldprotect) {
        return VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &out_oldprotect) != 0;
    }

    struct scoped_unprotect {
        void*  addr;
        size_t size;
        DWORD  dwOldProtect;
        bool   bUnprotected;

        scoped_unprotect(void* addr, size_t size)
            : addr(addr), size(size), dwOldProtect(0), bUnprotected(false) {
            if (size > 0)
                bUnprotected = UnprotectMemory(addr, size, dwOldProtect);
        }

        ~scoped_unprotect() {
            if (bUnprotected)
                ProtectMemory(addr, size, dwOldProtect);
        }
    };

    inline void WriteMemoryRaw(uintptr_t addr, void* value, size_t size, bool vp) {
        scoped_unprotect xprotect((void*)addr, vp ? size : 0);
        memcpy((void*)addr, value, size);
    }

    inline void ReadMemoryRaw(uintptr_t addr, void* ret, size_t size, bool vp) {
        scoped_unprotect xprotect((void*)addr, vp ? size : 0);
        memcpy(ret, (void*)addr, size);
    }

    inline void MemoryFill(uintptr_t addr, uint8_t value, size_t size, bool vp) {
        scoped_unprotect xprotect((void*)addr, vp ? size : 0);
        memset((void*)addr, value, size);
    }

    template <class T> inline T ReadMemory(uintptr_t addr, bool vp = false) {
        T                value;
        scoped_unprotect xprotect((void*)addr, vp ? sizeof(T) : 0);
        value = *(T*)addr;
        return value;
    }

    template <class T> inline void WriteMemory(uintptr_t addr, T value, bool vp = false) {
        scoped_unprotect xprotect((void*)addr, vp ? sizeof(T) : 0);
        *(T*)addr = value;
    }

    inline uintptr_t GetAbsoluteOffset(int rel_value, uintptr_t end_of_instruction) {
        return end_of_instruction + rel_value;
    }

    inline int32_t GetRelativeOffset(uintptr_t abs_value, uintptr_t end_of_instruction) {
        return (int32_t)(abs_value - end_of_instruction);
    }

    inline uintptr_t ReadRelativeOffset(uintptr_t at, size_t sizeof_addr = 4, bool vp = true) {
        switch (sizeof_addr) {
            case 1: return GetAbsoluteOffset(ReadMemory<int8_t>(at, vp), at + sizeof_addr);
            case 2: return GetAbsoluteOffset(ReadMemory<int16_t>(at, vp), at + sizeof_addr);
            case 4: return GetAbsoluteOffset(ReadMemory<int32_t>(at, vp), at + sizeof_addr);
        }
        return 0;
    }

    inline void MakeRelativeOffset(uintptr_t at, uintptr_t dest, size_t sizeof_addr = 4,
                                   bool vp = true) {
        switch (sizeof_addr) {
            case 1:
                WriteMemory<int8_t>(
                    at, static_cast<int8_t>(GetRelativeOffset(dest, at + sizeof_addr)), vp);
                break;
            case 2:
                WriteMemory<int16_t>(
                    at, static_cast<int16_t>(GetRelativeOffset(dest, at + sizeof_addr)), vp);
                break;
            case 4:
                WriteMemory<int32_t>(
                    at, static_cast<int32_t>(GetRelativeOffset(dest, at + sizeof_addr)), vp);
                break;
        }
    }

    inline uintptr_t GetBranchDestination(uintptr_t at, bool vp = true) {
        switch (ReadMemory<uint8_t>(at, vp)) {

            case 0x48:
            case 0x4C:
                switch (ReadMemory<uint8_t>(at + 1, vp)) {
                    case 0x8B: // mov
                    case 0x8D: // lea
                        switch (ReadMemory<uint8_t>(at + 2, vp)) {
                            case 0x0D:
                            case 0x1D:
                            case 0x15: return ReadRelativeOffset(at + 3, 4, vp);
                        }
                        break;
                }
                break;

            case 0xE8: // call rel
            case 0xE9: // jmp rel
                return ReadRelativeOffset(at + 1, 4, vp);

            case 0xFF:
            case 0x0F:
                switch (ReadMemory<uint8_t>(at + 1, vp)) {
                    case 0x15: // call dword ptr [addr]
                    case 0x25: // jmp dword ptr [addr]
                    case 0x85: // jne
                    case 0x8D: // jge
                    case 0x84: // jz
                    case 0x8E: // jle
                    case 0x82: // jb
                    case 0x8C: // jl
                        return ReadRelativeOffset(at + 2, 4, vp);
                }
                break;
        }
        return 0;
    }

    template <typename T> inline void MakeJMP(uintptr_t at, T dest, bool vp = true) {
        uintptr_t destAddr = (uintptr_t)dest;
        int64_t   offset   = (int64_t)destAddr - (int64_t)(at + 5);

        if (offset > 0x7FFFFFFF || offset < -0x7FFFFFFF) {
            // Far jump: FF 25 00 00 00 00 [8-byte addr]
            WriteMemory<uint8_t>(at, 0xFF, vp);
            WriteMemory<uint8_t>(at + 1, 0x25, vp);
            WriteMemory<uint32_t>(at + 2, 0x0, vp);
            WriteMemory<uint64_t>(at + 6, destAddr, vp);
        } else {
            WriteMemory<uint8_t>(at, 0xE9, vp);
            MakeRelativeOffset(at + 1, destAddr, 4, vp);
        }
    }

    template <typename T> inline void MakeCALL(uintptr_t at, T dest, bool vp = true) {
        uintptr_t destAddr = (uintptr_t)dest;
        int64_t   offset   = (int64_t)destAddr - (int64_t)(at + 5);

        if (offset > 0x7FFFFFFF || offset < -0x7FFFFFFF) {
            // Far call: FF 15 00 00 00 00 [8-byte addr]
            WriteMemory<uint8_t>(at, 0xFF, vp);
            WriteMemory<uint8_t>(at + 1, 0x15, vp);
            WriteMemory<uint32_t>(at + 2, 0x0, vp);
            WriteMemory<uint64_t>(at + 6, destAddr, vp);
        } else {
            WriteMemory<uint8_t>(at, 0xE8, vp);
            MakeRelativeOffset(at + 1, destAddr, 4, vp);
        }
    }

    inline void MakeNOP(uintptr_t at, size_t count = 1, bool vp = true) {
        MemoryFill(at, 0x90, count, vp);
    }
} // namespace Injector
