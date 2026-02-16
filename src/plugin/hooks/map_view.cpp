#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace MapView {
    extern "C" {
    void      mapViewProc1V137();
    void      mapViewProc2V137();
    void      mapViewProc3V137();
    void      mapViewProc4V137();
    uintptr_t mapViewProc1ReturnAddress;
    uintptr_t mapViewProc2ReturnAddress;
    uintptr_t mapViewProc3ReturnAddress;
    uintptr_t mapViewProc4ReturnAddress;
    uintptr_t mapViewProc3CallAddress;
    }

    bool mapViewProc1Injector() {
        // movss   [rbp+1060h+var_10C0], xmm3
        BytePattern::temp_instance().find_pattern("F3 0F 11 5D A0 41 0F B6 04 01 49 8B 14 C7");
        if (BytePattern::temp_instance().has_size(1, "char fetch in processing loop 2")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            mapViewProc1ReturnAddress = address + 0x11;

            Injector::MakeJMP(address, mapViewProc1V137, true);
            return false;
        }
        return true;
    }

    bool mapViewProc2Injector() {
        // movzx   eax, byte ptr [r15+rax]
        BytePattern::temp_instance().find_pattern("41 0F B6 04 07 4D 8B 9C C1 20 01 00 00");
        if (BytePattern::temp_instance().has_size(1, "char fetch in processing loop 1")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            mapViewProc2ReturnAddress = address + 0x10;

            Injector::MakeJMP(address, mapViewProc2V137, true);
            return false;
        }
        return true;
    }

    bool mapViewProc3Injector() {
        // movzx   r8d, byte ptr [r15+rax]
        BytePattern::temp_instance().find_pattern("45 0F B6 04 07 BA 01 00 00 00");
        if (BytePattern::temp_instance().has_size(1, "char copy in processing loop 1")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            // call {sub_xxxxx}
            mapViewProc3CallAddress = Injector::GetBranchDestination(address + 0x0F);

            // nop
            mapViewProc3ReturnAddress = address + 0x14;

            Injector::MakeJMP(address, mapViewProc3V137, true);
            return false;
        }
        return true;
    }

    bool mapViewProc4Injector() {
        // lea     rcx, [rsp+1160h+var_10E8]
        BytePattern::temp_instance().find_pattern("48 8D 4C 24 78 48 83 FB 10 48 0F 43 CE");
        if (BytePattern::temp_instance().has_size(1, "kerning")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            mapViewProc4ReturnAddress = address + 0x23;

            Injector::MakeJMP(address, mapViewProc4V137, true);
            return false;
        }
        return true;
    }

    HookResult Init(const RunOptions& options) {
        HookResult result("MapView");

        result.add("mapViewProc1Injector", mapViewProc1Injector());
        result.add("mapViewProc2Injector", mapViewProc2Injector());
        result.add("mapViewProc3Injector", mapViewProc3Injector());
        result.add("mapViewProc4Injector", mapViewProc4Injector());

        return result;
    }
} // namespace MapView
