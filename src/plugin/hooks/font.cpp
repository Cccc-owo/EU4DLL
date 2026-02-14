#include "../plugin_64.h"

namespace Font {

	extern "C" {
		void fontBufferHeapZeroClear();
		uintptr_t fontBufferHeapZeroClearReturnAddress;
		uintptr_t fontBufferHeapZeroClearHeepAllocJmpAddress;
		uintptr_t fontBufferHeapZeroClearHeapJmpAddress;
	}

	DllError charCodePointLimiterPatchInjector() {
		DllError e = {};

		// cmp     edi, 0FFh
		BytePattern::temp_instance().find_pattern("81 FF FF 00 00 00 0F 87 6F 02 00 00 83");
		if (BytePattern::temp_instance().has_size(1, "Char Code Point Limiter")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();
			Injector::WriteMemory<uint8_t>(address + 3, 0xFF, true);
		}
		else {
			e.font.unmatchdCharCodePointLimiterPatchInjector = true;
		}

		return e;
	}

	DllError fontBufferHeapZeroClearInjector() {
		DllError e = {};

		// mov rcx,cs:hHeap
		BytePattern::temp_instance().find_pattern("48 8B 0D ? ? ? ? 4C 8B C3 33 D2");
		if (BytePattern::temp_instance().has_size(1, "Font buffer heap zero clear")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			fontBufferHeapZeroClearHeapJmpAddress = Injector::GetBranchDestination(address + 0x0);
			fontBufferHeapZeroClearHeepAllocJmpAddress = Injector::GetBranchDestination(address + 0xC);
			fontBufferHeapZeroClearReturnAddress = address + 0x15;

			Injector::MakeJMP(address, fontBufferHeapZeroClear, true);
		}
		else {
			e.font.unmatchdFontBufferHeapZeroClearInjector = true;
		}

		return e;
	}

	DllError fontBufferClear1Injector() {
		DllError e = {};

		BytePattern::temp_instance().find_pattern("BA 88 3D 00 00 48 8B 4D");
		if (BytePattern::temp_instance().has_size(1, "Font buffer clear")) {
			Injector::WriteMemory<uint8_t>(BytePattern::temp_instance().get_first().address(0x3), 0x10, true);
		}
		else {
			e.font.unmatchdFontBufferClear1Injector = true;
		}

		return e;
	}

	DllError fontBufferClear2Injector() {
		DllError e = {};

		BytePattern::temp_instance().find_pattern("BA 88 3D 00 00 48 8B CB E8");
		if (BytePattern::temp_instance().has_size(1, "Font buffer clear")) {
			Injector::WriteMemory<uint8_t>(BytePattern::temp_instance().get_first().address(0x3), 0x10, true);
		}
		else {
			e.font.unmatchdFontBufferClear2Injector = true;
		}

		return e;
	}

	DllError fontBufferExpansionInjector() {
		DllError e = {};

		BytePattern::temp_instance().find_pattern("B9 88 3D 00 00");
		if (BytePattern::temp_instance().has_size(1, "Font buffer expansion")) {
			Injector::WriteMemory<uint8_t>(BytePattern::temp_instance().get_first().address(0x3), 0x10, true);
		}
		else {
			e.font.unmatchdFontBufferExpansionInjector = true;
		}

		return e;
	}

	DllError fontSizeLimitInjector() {
		DllError e = {};

		BytePattern::temp_instance().find_pattern("41 81 FE 00 00 00 01");
		if (BytePattern::temp_instance().has_size(1, "Font size limit")) {
			Injector::WriteMemory<uint8_t>(BytePattern::temp_instance().get_first().address(0x6), 0x04, true);
		}
		else {
			e.font.unmatchdFontSizeLimitInjector = true;
		}

		return e;
	}

	DllError Init(RunOptions options) {
		DllError result = {};

		result |= charCodePointLimiterPatchInjector();
		result |= fontBufferHeapZeroClearInjector();
		result |= fontBufferClear1Injector();
		result |= fontBufferClear2Injector();
		result |= fontBufferExpansionInjector();
		result |= fontSizeLimitInjector();

		return result;
	}
}
