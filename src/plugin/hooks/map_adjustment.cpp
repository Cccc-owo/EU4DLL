#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace MapAdjustment {
    extern "C" {
    void      mapAdjustmentProc1V137A();
    void      mapAdjustmentProc1V137B();
    void      mapAdjustmentProc2V137();
    void      mapAdjustmentProc3V137();
    void      mapAdjustmentProc4V137();
    uintptr_t mapAdjustmentProc1ReturnAddressA;
    uintptr_t mapAdjustmentProc1ReturnAddressB;
    uintptr_t mapAdjustmentProc1CallAddressA;
    uintptr_t mapAdjustmentProc1CallAddressB;
    uintptr_t mapAdjustmentProc2ReturnAddress;
    uintptr_t mapAdjustmentProc3ReturnAddress1;
    uintptr_t mapAdjustmentProc3ReturnAddress2;
    uintptr_t mapAdjustmentProc4ReturnAddress;
    }

    bool mapAdjustmentProc1Injector() {
        // movsx   ecx, byte ptr [rax+rbp]
        BytePattern::temp_instance().find_pattern("0F BE 0C 28 48 8D 1C 28 E8 ? ? ? ? FF C7");
        if (BytePattern::temp_instance().has_size(2, "cancel map text uppercasing")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            mapAdjustmentProc1CallAddressA   = Injector::GetBranchDestination(address + 0x08);
            mapAdjustmentProc1ReturnAddressA = address + 0x13;

            Injector::MakeJMP(address, mapAdjustmentProc1V137A, true);

            address = BytePattern::temp_instance().get_second().address();

            mapAdjustmentProc1CallAddressB   = Injector::GetBranchDestination(address + 0x08);
            mapAdjustmentProc1ReturnAddressB = address + 0x13;

            Injector::MakeJMP(address, mapAdjustmentProc1V137B, true);
            return false;
        }
        return true;
    }

    bool mapAdjustmentProc2Injector() {
        // lea     rax, [rbp+230h+Block]
        BytePattern::temp_instance().find_pattern(
            "48 8D 85 90 00 00 00 49 83 FD 10 48 0F 43 C6 0F B6 04 18");
        if (BytePattern::temp_instance().has_size(1, "char check fix")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            mapAdjustmentProc2ReturnAddress = address + 0x2B;

            Injector::MakeJMP(address, mapAdjustmentProc2V137, true);
            return false;
        }
        return true;
    }

    bool mapAdjustmentProc3Injector() {
        bool failed = false;

        // lea     rdx, [rbp+230h+var_1C0]
        BytePattern::temp_instance().find_pattern("48 8D 55 70 48 83 FF 10");
        if (BytePattern::temp_instance().has_size(1, "copy after char check")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            mapAdjustmentProc3ReturnAddress1 = address + 0x18;

            Injector::MakeJMP(address, mapAdjustmentProc3V137, true);
        } else {
            failed = true;
        }

        // mov     rax, [rbp+230h+arg_0]
        BytePattern::temp_instance().find_pattern("48 8B 85 40 02 00 00 48 8B 48 30");
        if (BytePattern::temp_instance().has_size(1, "copy after char check return address 2")) {
            mapAdjustmentProc3ReturnAddress2 = BytePattern::temp_instance().get_first().address();
        } else {
            failed = true;
        }

        return failed;
    }

    bool mapAdjustmentProc4Injector() {
        //  lea     rax, [rbp+230h+Block]
        BytePattern::temp_instance().find_pattern(
            "48 8D 85 90 00 00 00 49 83 FD 10 48 0F 43 C6 0F B6 04 08");
        if (BytePattern::temp_instance().has_size(1, "char fetch fix")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            mapAdjustmentProc4ReturnAddress = address + 0x13;

            Injector::MakeJMP(address, mapAdjustmentProc4V137, true);
            return false;
        }
        return true;
    }

    HookResult Init(const RunOptions& options) {
        HookResult result("MapAdjustment");

        result.add("mapAdjustmentProc1Injector", mapAdjustmentProc1Injector());
        result.add("mapAdjustmentProc2Injector", mapAdjustmentProc2Injector());
        result.add("mapAdjustmentProc3Injector", mapAdjustmentProc3Injector());
        result.add("mapAdjustmentProc4Injector", mapAdjustmentProc4Injector());
        // proc5 is no-op for v1.37 (handled by localization yml)

        return result;
    }
} // namespace MapAdjustment
