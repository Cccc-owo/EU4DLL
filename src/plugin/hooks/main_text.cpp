#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace MainText {
	extern "C" {
		void mainTextProc1V137();
		void mainTextProc2V137();
		void mainTextProc3();
		void mainTextProc4V137();
		uintptr_t mainTextProc1ReturnAddress;
		uintptr_t mainTextProc2ReturnAddress;
		uintptr_t mainTextProc2BufferAddress;
		uintptr_t mainTextProc3ReturnAddress1;
		uintptr_t mainTextProc3ReturnAddress2;
		uintptr_t mainTextProc4ReturnAddress;
	}

	bool mainTextProc1Injector() {
		// movzx   r8d, byte ptr [rcx+r9]
		BytePattern::temp_instance().find_pattern("46 0F B6 04 09");
		if (BytePattern::temp_instance().has_size(1, "テキスト処理ループ２の文字取得修正")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mainTextProc1ReturnAddress = address + 0x16;

			Injector::MakeJMP(address, mainTextProc1V137, true);
			return false;
		}
		return true;
	}

	bool mainTextProc2Injector() {
		// movsxd  r9, edi
		BytePattern::temp_instance().find_pattern("4C 63 CF 48 8B 55 F8 4C 03 CA 48 63 CE");
		if (BytePattern::temp_instance().has_size(1, "テキスト処理ループ１のカウント処理修正")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mainTextProc2ReturnAddress = address + 0x1E;

			// lea r11, {unk_XXXXX}
			mainTextProc2BufferAddress = Injector::GetBranchDestination(address + 0x11);

			Injector::MakeJMP(address, mainTextProc2V137, true);
			return false;
		}
		return true;
	}

	bool mainTextProc3Injector() {
		bool failed = false;

		// cmp cs:byte_xxxxx, 0
		BytePattern::temp_instance().find_pattern("80 3D ? ? ? ? 00 0F 84 5F 01");
		if (BytePattern::temp_instance().has_size(1, "テキスト処理ループ１の改行処理の戻り先２取得")) {
			mainTextProc3ReturnAddress2 = BytePattern::temp_instance().get_first().address();
		}
		else {
			failed = true;
		}

		// cmp word ptr [rcx+6],0
		BytePattern::temp_instance().find_pattern("66 83 79 06 00 0F 85");
		if (BytePattern::temp_instance().has_size(1, "テキスト処理ループ１の改行処理を修正")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mainTextProc3ReturnAddress1 = address + 0x12;

			Injector::MakeJMP(address, mainTextProc3, true);
		}
		else {
			failed = true;
		}

		return failed;
	}

	bool mainTextProc4Injector() {
		// movzx   eax, byte ptr [r9]
		BytePattern::temp_instance().find_pattern("41 0F B6 01 49 8B 8C C6 20 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "テキスト処理ループ１の文字取得修正")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mainTextProc4ReturnAddress = address + 0x13;

			Injector::MakeJMP(address, mainTextProc4V137, true);
			return false;
		}
		return true;
	}

	HookResult Init(RunOptions options) {
		HookResult result("MainText");

		result.add("mainTextProc1Injector", mainTextProc1Injector());
		result.add("mainTextProc2Injector", mainTextProc2Injector());
		result.add("mainTextProc3Injector", mainTextProc3Injector());
		result.add("mainTextProc4Injector", mainTextProc4Injector());

		return result;
	}
}
