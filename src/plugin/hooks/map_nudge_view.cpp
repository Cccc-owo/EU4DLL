#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace MapNudgeView {
	extern "C" {
		void mapNudgeViewProc1V137();
		uintptr_t mapNudgeViewProc1ReturnAddress;
	}

	bool mapNudgeViewProc1Injector() {
		// movzx   eax, byte ptr [rcx+rax]
		BytePattern::temp_instance().find_pattern("0F B6 04 01 49 8B 94 C4 20 01 00 00 48 85 D2");
		if (BytePattern::temp_instance().has_size(1, "char fetch")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapNudgeViewProc1ReturnAddress = address + 0x0F;

			Injector::MakeJMP(address, mapNudgeViewProc1V137, true);
			return false;
		}
		return true;
	}

	HookResult Init(RunOptions options) {
		HookResult result("MapNudgeView");

		result.add("mapNudgeViewProc1Injector", mapNudgeViewProc1Injector());

		return result;
	}
}
