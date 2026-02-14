#include "../plugin_64.h"
#include "../escape_tool.h"

namespace MapAdjustment {
	extern "C" {
		void mapAdjustmentProc1V137A();
		void mapAdjustmentProc1V137B();
		void mapAdjustmentProc2V137();
		void mapAdjustmentProc3V137();
		void mapAdjustmentProc4V137();
		uintptr_t mapAdjustmentProc1ReturnAddressA;
		uintptr_t mapAdjustmentProc1ReturnAddressB;
		uintptr_t mapAdjustmentProc1CallAddressA;
		uintptr_t mapAdjustmentProc1CallAddressB;
		uintptr_t mapAdjustmentProc2ReturnAddress;
		uintptr_t mapAdjustmentProc3ReturnAddress1;
		uintptr_t mapAdjustmentProc3ReturnAddress2;
		uintptr_t mapAdjustmentProc4ReturnAddress;
	}

	DllError mapAdjustmentProc1Injector() {
		DllError e = {};

		// movsx   ecx, byte ptr [rax+rbp]
		BytePattern::temp_instance().find_pattern("0F BE 0C 28 48 8D 1C 28 E8 ? ? ? ? FF C7");
		if (BytePattern::temp_instance().has_size(2, "マップ文字の大文字化キャンセル")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapAdjustmentProc1CallAddressA = Injector::GetBranchDestination(address + 0x08);
			mapAdjustmentProc1ReturnAddressA = address + 0x13;

			Injector::MakeJMP(address, mapAdjustmentProc1V137A, true);

			address = BytePattern::temp_instance().get_second().address();

			mapAdjustmentProc1CallAddressB = Injector::GetBranchDestination(address + 0x08);
			mapAdjustmentProc1ReturnAddressB = address + 0x13;

			Injector::MakeJMP(address, mapAdjustmentProc1V137B, true);
		}
		else {
			e.mapAdjustment.unmatchdMapAdjustmentProc1Injector = true;
		}

		return e;
	}

	DllError mapAdjustmentProc2Injector() {
		DllError e = {};

		// lea     rax, [rbp+230h+Block]
		BytePattern::temp_instance().find_pattern("48 8D 85 90 00 00 00 49 83 FD 10 48 0F 43 C6 0F B6 04 18");
		if (BytePattern::temp_instance().has_size(1, "文字チェック修正")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapAdjustmentProc2ReturnAddress = address + 0x2B;

			Injector::MakeJMP(address, mapAdjustmentProc2V137, true);
		}
		else {
			e.mapAdjustment.unmatchdMapAdjustmentProc2Injector = true;
		}

		return e;
	}

	DllError mapAdjustmentProc3Injector() {
		DllError e = {};

		// lea     rdx, [rbp+230h+var_1C0]
		BytePattern::temp_instance().find_pattern("48 8D 55 70 48 83 FF 10");
		if (BytePattern::temp_instance().has_size(1, "文字チェックの後のコピー処理")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapAdjustmentProc3ReturnAddress1 = address + 0x18;

			Injector::MakeJMP(address, mapAdjustmentProc3V137, true);
		}
		else {
			e.mapAdjustment.unmatchdMapAdjustmentProc3Injector = true;
		}

		// mov     rax, [rbp+230h+arg_0]
		BytePattern::temp_instance().find_pattern("48 8B 85 40 02 00 00 48 8B 48 30");
		if (BytePattern::temp_instance().has_size(1, "文字チェックの後のコピー処理の戻り先２")) {
			mapAdjustmentProc3ReturnAddress2 = BytePattern::temp_instance().get_first().address();
		}
		else {
			e.mapAdjustment.unmatchdMapAdjustmentProc3Injector = true;
		}

		return e;
	}

	DllError mapAdjustmentProc4Injector() {
		DllError e = {};

		//  lea     rax, [rbp+230h+Block]
		BytePattern::temp_instance().find_pattern("48 8D 85 90 00 00 00 49 83 FD 10 48 0F 43 C6 0F B6 04 08");
		if (BytePattern::temp_instance().has_size(1, "文字取得処理修正")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapAdjustmentProc4ReturnAddress = address + 0x13;

			Injector::MakeJMP(address, mapAdjustmentProc4V137, true);
		}
		else {
			e.mapAdjustment.unmatchdMapAdjustmentProc4Injector = true;
		}

		return e;
	}

	DllError Init(RunOptions options) {
		DllError result = {};

		result |= mapAdjustmentProc1Injector();
		result |= mapAdjustmentProc2Injector();
		result |= mapAdjustmentProc3Injector();
		result |= mapAdjustmentProc4Injector();
		// proc5 is no-op for v1.37 (handled by localization yml)

		return result;
	}
}
