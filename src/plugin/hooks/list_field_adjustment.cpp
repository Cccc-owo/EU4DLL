#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace ListFieldAdjustment {
	extern "C" {
		void listFieldAdjustmentProc1V137();
		void listFieldAdjustmentProc2V137();
		void listFieldAdjustmentProc3V137A();
		void listFieldAdjustmentProc3V137B();
		uintptr_t listFieldAdjustmentProc1ReturnAddress;
		uintptr_t listFieldAdjustmentProc2ReturnAddress;
		uintptr_t listFieldAdjustmentProc3ReturnAddressA;
		uintptr_t listFieldAdjustmentProc3ReturnAddressB;
	}

	bool listFieldAdjustmentProc1Injector() {
		// mov     r8, [rbp+100h+var_148]
		BytePattern::temp_instance().find_pattern("4C 8B 45 B8 F3 41 0F 10 B0 48 08 00 00");
		if (BytePattern::temp_instance().has_size(1, "font loading")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			listFieldAdjustmentProc1ReturnAddress = address + 0x18;

			Injector::MakeJMP(address, listFieldAdjustmentProc1V137, true);
			return false;
		}
		return true;
	}

	bool listFieldAdjustmentProc2Injector() {
		// inc     ebx
		BytePattern::temp_instance().find_pattern("FF C3 8B F3 8B 4F 10 44 0F B6 8D 48 01 00 00 45 33 D2");
		if (BytePattern::temp_instance().has_size(1, "advance counter")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			listFieldAdjustmentProc2ReturnAddress = address + 0x14;

			Injector::MakeJMP(address, listFieldAdjustmentProc2V137, true);
			return false;
		}
		return true;
	}

	bool listFieldAdjustmentProc3Injector() {
		// mov     rcx, [rax+rcx*8]
		BytePattern::temp_instance().find_pattern("48 8B 0C C8 44 8B 0C 91 45 33 C0 48 8D 55 98");
		if (BytePattern::temp_instance().has_size(2, "string truncation")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			listFieldAdjustmentProc3ReturnAddressA = address + 0x12;

			Injector::MakeJMP(address, listFieldAdjustmentProc3V137A, true);

			address = BytePattern::temp_instance().get_second().address();

			listFieldAdjustmentProc3ReturnAddressB = address + 0x12;

			Injector::MakeJMP(address, listFieldAdjustmentProc3V137B, true);
			return false;
		}
		return true;
	}

	HookResult Init(const RunOptions& options) {
		HookResult result("ListFieldAdjustment");

		result.add("listFieldAdjustmentProc1Injector", listFieldAdjustmentProc1Injector());
		result.add("listFieldAdjustmentProc2Injector", listFieldAdjustmentProc2Injector());
		result.add("listFieldAdjustmentProc3Injector", listFieldAdjustmentProc3Injector());

		return result;
	}
}
