#include "../plugin_64.h"

namespace MapNudgeView {
	extern "C" {
		void mapNudgeViewProc1V137();
		uintptr_t mapNudgeViewProc1ReturnAddress;
	}

	DllError mapNudgeViewProc1Injector() {
		DllError e = {};

		// movzx   eax, byte ptr [rcx+rax]
		BytePattern::temp_instance().find_pattern("0F B6 04 01 49 8B 94 C4 20 01 00 00 48 85 D2");
		if (BytePattern::temp_instance().has_size(1, "文字取得処理")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapNudgeViewProc1ReturnAddress = address + 0x0F;

			Injector::MakeJMP(address, mapNudgeViewProc1V137, true);
		}
		else {
			e.mapNudge.unmatchdMapNudgeViewProc1Injector = true;
		}

		return e;
	}

	DllError Init(RunOptions options) {
		DllError result = {};

		result |= mapNudgeViewProc1Injector();

		return result;
	}
}
