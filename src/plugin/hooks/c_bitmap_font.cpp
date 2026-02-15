#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace CBitmapFont {
	extern "C" {
		void cBitmapFontProc1V137();
		void cBitmapFontProc2V137();
		uintptr_t cBitmapFontProc1ReturnAddress;
		uintptr_t cBitmapFontProc2ReturnAddress;
	}

	bool cBitmapFontProc1Injector() {
		// movzx   eax, byte ptr [rdi+rax]
		BytePattern::temp_instance().find_pattern("0F B6 04 07 49 8B 8C C6 20 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "CBitmapFont文字取得1")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			cBitmapFontProc1ReturnAddress = address + 0x0F;

			Injector::MakeJMP(address, cBitmapFontProc1V137, true);
			return false;
		}
		return true;
	}

	bool cBitmapFontProc2Injector() {
		// movss   xmm6, dword ptr [r14+848h]
		BytePattern::temp_instance().find_pattern("F3 41 0F 10 B6 48 08 00 00 0F B6 04 02 4D 8B 3C C6");
		if (BytePattern::temp_instance().has_size(1, "CBitmapFont文字取得2")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			cBitmapFontProc2ReturnAddress = address + 0x14;

			Injector::MakeJMP(address, cBitmapFontProc2V137, true);
			return false;
		}
		return true;
	}

	HookResult Init(RunOptions options) {
		HookResult result("CBitmapFont");

		result.add("cBitmapFontProc1Injector", cBitmapFontProc1Injector());
		result.add("cBitmapFontProc2Injector", cBitmapFontProc2Injector());

		return result;
	}
}
