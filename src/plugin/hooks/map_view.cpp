#include "../plugin_64.h"

namespace MapView {
	extern "C" {
		void mapViewProc1V137();
		void mapViewProc2V137();
		void mapViewProc3V137();
		void mapViewProc4V137();
		uintptr_t mapViewProc1ReturnAddress;
		uintptr_t mapViewProc2ReturnAddress;
		uintptr_t mapViewProc3ReturnAddress;
		uintptr_t mapViewProc4ReturnAddress;
		uintptr_t mapViewProc3CallAddress;
	}

	DllError mapViewProc1Injector() {
		DllError e = {};

		// movss   [rbp+1060h+var_10C0], xmm3
		BytePattern::temp_instance().find_pattern("F3 0F 11 5D A0 41 0F B6 04 01 49 8B 14 C7");
		if (BytePattern::temp_instance().has_size(1, "処理ループ２の文字取得処理")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapViewProc1ReturnAddress = address + 0x11;

			Injector::MakeJMP(address, mapViewProc1V137, true);
		}
		else {
			e.mapView.unmatchdMapViewProc1Injector = true;
		}

		return e;
	}

	DllError mapViewProc2Injector() {
		DllError e = {};

		// movzx   eax, byte ptr [r15+rax]
		BytePattern::temp_instance().find_pattern("41 0F B6 04 07 4D 8B 9C C1 20 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "処理ループ１の文字取得処理")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapViewProc2ReturnAddress = address + 0x10;

			Injector::MakeJMP(address, mapViewProc2V137, true);
		}
		else {
			e.mapView.unmatchdMapViewProc2Injector = true;
		}

		return e;
	}

	DllError mapViewProc3Injector() {
		DllError e = {};

		// movzx   r8d, byte ptr [r15+rax]
		BytePattern::temp_instance().find_pattern("45 0F B6 04 07 BA 01 00 00 00");
		if (BytePattern::temp_instance().has_size(1, "処理ループ１の文字コピー")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			// call {sub_xxxxx}
			mapViewProc3CallAddress = Injector::GetBranchDestination(address + 0x0F);

			// nop
			mapViewProc3ReturnAddress = address + 0x14;

			Injector::MakeJMP(address, mapViewProc3V137, true);
		}
		else {
			e.mapView.unmatchdMapViewProc3Injector = true;
		}

		return e;
	}

	DllError mapViewProc4Injector() {
		DllError e = {};

		// lea     rcx, [rsp+1160h+var_10E8]
		BytePattern::temp_instance().find_pattern("48 8D 4C 24 78 48 83 FB 10 48 0F 43 CE");
		if (BytePattern::temp_instance().has_size(1, "kerning")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapViewProc4ReturnAddress = address + 0x23;

			Injector::MakeJMP(address, mapViewProc4V137, true);
		}
		else {
			e.mapView.unmatchdMapViewProc4Injector = true;
		}

		return e;
	}

	DllError Init(RunOptions options) {
		DllError result = {};

		result |= mapViewProc1Injector();
		result |= mapViewProc2Injector();
		result |= mapViewProc3Injector();
		result |= mapViewProc4Injector();

		return result;
	}
}
