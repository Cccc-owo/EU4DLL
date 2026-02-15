#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace MapJustify {
	extern "C" {
		void mapJustifyProc1V137();
		void mapJustifyProc2V137();
		void mapJustifyProc4V137();
		uintptr_t mapJustifyProc1ReturnAddress1;
		uintptr_t mapJustifyProc1ReturnAddress2;
		uintptr_t mapJustifyProc2ReturnAddress;
		uintptr_t mapJustifyProc4ReturnAddress;
	}

	bool mapJustifyProc1Injector() {
		// movzx   eax, byte ptr [rcx+rax]
		BytePattern::temp_instance().find_pattern("0F B6 04 01 88 85 08 08 00 00 F3 44 0F 10 A2 48 08 00 00");
		if (BytePattern::temp_instance().has_size(1, "char fetch")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapJustifyProc1ReturnAddress1 = address + 0x1A;

			Injector::MakeJMP(address, mapJustifyProc1V137, true);
			return false;
		}
		return true;
	}

	bool mapJustifyProc2Injector() {
		// movd    xmm6, esi
		BytePattern::temp_instance().find_pattern("66 0F 6E F6 0F 5B F6 48 8B 85 68 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "single char display adjustment")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapJustifyProc2ReturnAddress = address + 0x14;

			Injector::MakeJMP(address, mapJustifyProc2V137, true);
			return false;
		}
		return true;
	}

	bool mapJustifyProc4Injector() {
		// inc     esi
		BytePattern::temp_instance().find_pattern("FF C6 89 B5 E8 07 00 00 48 FF C1");
		if (BytePattern::temp_instance().has_size(1, "counter processing")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			mapJustifyProc4ReturnAddress = address + 0x12;

			Injector::MakeJMP(address, mapJustifyProc4V137, true);
			return false;
		}
		return true;
	}

	HookResult Init(const RunOptions& options) {
		HookResult result("MapJustify");

		result.add("mapJustifyProc1Injector", mapJustifyProc1Injector());
		result.add("mapJustifyProc2Injector", mapJustifyProc2Injector());
		result.add("mapJustifyProc4Injector", mapJustifyProc4Injector());

		return result;
	}
}
