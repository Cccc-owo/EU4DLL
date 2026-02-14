#include "../plugin_64.h"
#include "../escape_tool.h"

namespace Input {
	extern "C" {
		void inputProc1V137();
		void inputProc2V137();
		uintptr_t inputProc1ReturnAddress1;
		uintptr_t inputProc1ReturnAddress2;
		uintptr_t inputProc1CallAddress;
		uintptr_t inputProc2ReturnAddress;
	}

	DllError inputProc1Injector() {
		DllError e = {};

		// mov     eax, dword ptr [rbp+120h+var_18C]
		BytePattern::temp_instance().find_pattern("8B 45 BC 32 DB 3C 80 73 05 0F B6 D8 EB 12");
		if (BytePattern::temp_instance().has_size(1, "入力した文字をutf8からエスケープ列へ変換する１")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			inputProc1CallAddress = (uintptr_t)utf8ToEscapedStr3;

			// mov     rax, [r15]
			inputProc1ReturnAddress1 = address + 0x20;

			Injector::MakeJMP(address, inputProc1V137, true);
		}
		else {
			e.input.unmatchdInputProc1Injector = true;
		}

		// call    qword ptr [rax+18h]
		BytePattern::temp_instance().find_pattern("FF 50 18 E9 ? ? ? ? 49 8B 07 45 33 C9");
		if (BytePattern::temp_instance().has_size(1, "入力した文字をutf8からエスケープ列へ変換する２")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();
			// jmp     loc_{xxxxx}
			inputProc1ReturnAddress2 = Injector::GetBranchDestination(address + 0x3);
		}
		else {
			e.input.unmatchdInputProc1Injector = true;
		}

		return e;
	}

	DllError inputProc2Injector() {
		DllError e = {};

		// movzx   eax, byte ptr [rcx+rsi]
		BytePattern::temp_instance().find_pattern("0F B6 04 31 3C 0A");
		if (BytePattern::temp_instance().has_size(1, "改行入力")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			inputProc2ReturnAddress = address + 0x0E;

			Injector::MakeJMP(address, inputProc2V137, true);
		}
		else {
			e.input.unmatchdInputProc2Injector = true;
		}

		return e;
	}

	DllError Init(RunOptions options) {
		DllError result = {};

		result |= inputProc1Injector();
		result |= inputProc2Injector();

		return result;
	}
}
