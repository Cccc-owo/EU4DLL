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

	DllError mapPopupProc1Injector() {
		DllError e = {};

		// lea     rcx, [rbp+370h+var_320]
		BytePattern::temp_instance().find_pattern("48 8D 4D 50 E8 ? ? ? ? 90 44 0F B6 C3 BA 01 00 00 00 48 8D 4D 50");
		if (BytePattern::temp_instance().has_size(3, "ループ１の文字列コピー")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			// call xxxx
			mapPopupProc1CallAddressA = Injector::GetBranchDestination(address + 0x4);

			// call xxxx
			mapPopupProc1CallAddressB = Injector::GetBranchDestination(address + 0x17);

			Injector::MakeJMP(address, mapPopupProc1V137, true);

			// nop
			mapPopupProc1ReturnAddress = address + 0x1C;
		}
		else {
			e.mapPopup.unmatchdMapPopupProc1Injector = true;
		}

		return e;
	}

	DllError mapPopupProc2Injector() {
		DllError e = {};

		BytePattern::temp_instance().find_pattern("0F B6 04 07 4D 8B A4 C7 20 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "ループ１の文字取得")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			Injector::MakeJMP(address, mapPopupProc2V137, true);

			mapPopupProc2ReturnAddress = address + 0xF;
		}
		else {
			e.mapPopup.unmatchdMapPopupProc2Injector = true;
		}

		return e;
	}

	DllError mapPopupProc3Injector() {
		DllError e = {};

		// movzx   eax, byte ptr [rsi+rax]
		BytePattern::temp_instance().find_pattern("0F B6 04 06 4D 8B AC C7 20 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "ループ２の文字取得")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			Injector::MakeJMP(address, mapPopupProc3V137, true);

			mapPopupProc3ReturnAddress = address + 0xF;
		}
		else {
			e.mapPopup.unmatchdMapPopupProc3Injector = true;
		}

		return e;
	}

	DllError Init(RunOptions options) {
		DllError result = {};

		result |= mapPopupProc1Injector();
		result |= mapPopupProc2Injector();
		result |= mapPopupProc3Injector();

		return result;
	}
}
