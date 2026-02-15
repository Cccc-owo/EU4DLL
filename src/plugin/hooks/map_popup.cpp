#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace MapPopup {
	extern "C" {
		void mapPopupProc1V137();
		void mapPopupProc2V137();
		void mapPopupProc3V137();
		uintptr_t mapPopupProc1ReturnAddress;
		uintptr_t mapPopupProc1CallAddressA;
		uintptr_t mapPopupProc1CallAddressB;
		uintptr_t mapPopupProc2ReturnAddress;
		uintptr_t mapPopupProc3ReturnAddress;
	}

	bool mapPopupProc1Injector() {
		// lea     rcx, [rbp+370h+var_320]
		BytePattern::temp_instance().find_pattern("48 8D 4D 50 E8 ? ? ? ? 90 44 0F B6 C3 BA 01 00 00 00 48 8D 4D 50");
		if (BytePattern::temp_instance().has_size(3, "string copy in loop 1")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			// call xxxx
			mapPopupProc1CallAddressA = Injector::GetBranchDestination(address + 0x4);

			// call xxxx
			mapPopupProc1CallAddressB = Injector::GetBranchDestination(address + 0x17);

			Injector::MakeJMP(address, mapPopupProc1V137, true);

			// nop
			mapPopupProc1ReturnAddress = address + 0x1C;
			return false;
		}
		return true;
	}

	bool mapPopupProc2Injector() {
		BytePattern::temp_instance().find_pattern("0F B6 04 07 4D 8B A4 C7 20 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "char fetch in loop 1")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			Injector::MakeJMP(address, mapPopupProc2V137, true);

			mapPopupProc2ReturnAddress = address + 0xF;
			return false;
		}
		return true;
	}

	bool mapPopupProc3Injector() {
		// movzx   eax, byte ptr [rsi+rax]
		BytePattern::temp_instance().find_pattern("0F B6 04 06 4D 8B AC C7 20 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "char fetch in loop 2")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			Injector::MakeJMP(address, mapPopupProc3V137, true);

			mapPopupProc3ReturnAddress = address + 0xF;
			return false;
		}
		return true;
	}

	HookResult Init(const RunOptions& options) {
		HookResult result("MapPopup");

		result.add("mapPopupProc1Injector", mapPopupProc1Injector());
		result.add("mapPopupProc2Injector", mapPopupProc2Injector());
		result.add("mapPopupProc3Injector", mapPopupProc3Injector());

		return result;
	}
}
