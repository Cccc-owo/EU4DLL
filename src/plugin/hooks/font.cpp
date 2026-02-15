#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace Font {

	extern "C" {
		void fontBufferHeapZeroClear();
		uintptr_t fontBufferHeapZeroClearReturnAddress;
		uintptr_t fontBufferHeapZeroClearHeepAllocJmpAddress;
		uintptr_t fontBufferHeapZeroClearHeapJmpAddress;
	}

	bool charCodePointLimiterPatchInjector() {
		// cmp     edi, 0FFh
		BytePattern::temp_instance().find_pattern("81 FF FF 00 00 00 0F 87 6F 02 00 00 83");
		if (BytePattern::temp_instance().has_size(1, "Char Code Point Limiter")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();
			Injector::WriteMemory<uint8_t>(address + 3, 0xFF, true);
			return false;
		}
		return true;
	}

	bool fontBufferHeapZeroClearInjector() {
		// mov rcx,cs:hHeap
		BytePattern::temp_instance().find_pattern("48 8B 0D ? ? ? ? 4C 8B C3 33 D2");
		if (BytePattern::temp_instance().has_size(1, "Font buffer heap zero clear")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			fontBufferHeapZeroClearHeapJmpAddress = Injector::GetBranchDestination(address + 0x0);
			fontBufferHeapZeroClearHeepAllocJmpAddress = Injector::GetBranchDestination(address + 0xC);
			fontBufferHeapZeroClearReturnAddress = address + 0x15;

			Injector::MakeJMP(address, fontBufferHeapZeroClear, true);
			return false;
		}
		return true;
	}

	bool fontBufferClear1Injector() {
		BytePattern::temp_instance().find_pattern("BA 88 3D 00 00 48 8B 4D");
		if (BytePattern::temp_instance().has_size(1, "Font buffer clear")) {
			Injector::WriteMemory<uint8_t>(BytePattern::temp_instance().get_first().address(0x3), 0x10, true);
			return false;
		}
		return true;
	}

	bool fontBufferClear2Injector() {
		BytePattern::temp_instance().find_pattern("BA 88 3D 00 00 48 8B CB E8");
		if (BytePattern::temp_instance().has_size(1, "Font buffer clear")) {
			Injector::WriteMemory<uint8_t>(BytePattern::temp_instance().get_first().address(0x3), 0x10, true);
			return false;
		}
		return true;
	}

	bool fontBufferExpansionInjector() {
		BytePattern::temp_instance().find_pattern("B9 88 3D 00 00");
		if (BytePattern::temp_instance().has_size(1, "Font buffer expansion")) {
			Injector::WriteMemory<uint8_t>(BytePattern::temp_instance().get_first().address(0x3), 0x10, true);
			return false;
		}
		return true;
	}

	bool fontSizeLimitInjector() {
		BytePattern::temp_instance().find_pattern("41 81 FE 00 00 00 01");
		if (BytePattern::temp_instance().has_size(1, "Font size limit")) {
			Injector::WriteMemory<uint8_t>(BytePattern::temp_instance().get_first().address(0x6), 0x04, true);
			return false;
		}
		return true;
	}

	HookResult Init(const RunOptions& options) {
		HookResult result("Font");

		result.add("charCodePointLimiterPatchInjector", charCodePointLimiterPatchInjector());
		result.add("fontBufferHeapZeroClearInjector", fontBufferHeapZeroClearInjector());
		result.add("fontBufferClear1Injector", fontBufferClear1Injector());
		result.add("fontBufferClear2Injector", fontBufferClear2Injector());
		result.add("fontBufferExpansionInjector", fontBufferExpansionInjector());
		result.add("fontSizeLimitInjector", fontSizeLimitInjector());

		return result;
	}
}
