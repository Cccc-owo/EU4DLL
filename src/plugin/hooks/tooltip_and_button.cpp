#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace TooltipAndButton {
	extern "C" {
		void tooltipAndButtonProc1V137();
		void tooltipAndButtonProc2V137();
		void tooltipAndButtonProc3V137();
		void tooltipAndButtonProc4V137();
		void tooltipAndButtonProc5V137();
		void tooltipAndButtonProc7V137();

		uintptr_t tooltipAndButtonProc1ReturnAddress;
		uintptr_t tooltipAndButtonProc1CallAddress;
		uintptr_t tooltipAndButtonProc2ReturnAddress;
		uintptr_t tooltipAndButtonProc3ReturnAddress;
		uintptr_t tooltipAndButtonProc4ReturnAddress1;
		uintptr_t tooltipAndButtonProc4ReturnAddress2;
		uintptr_t tooltipAndButtonProc4ReturnAddress3;
		uintptr_t tooltipAndButtonProc5ReturnAddress1;
		uintptr_t tooltipAndButtonProc5ReturnAddress2;
		uintptr_t tooltipAndButtonProc7ReturnAddress1;
		uintptr_t tooltipAndButtonProc7ReturnAddress2;
	}

	bool tooltipAndButtonProc1Injector() {
		// movzx   r8d, byte ptr [rax+rbx]
		BytePattern::temp_instance().find_pattern("44 0F B6 04 18 BA 01 00 00 00");
		if (BytePattern::temp_instance().has_size(1, "char copy in processing loop 1")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			tooltipAndButtonProc1CallAddress = Injector::GetBranchDestination(address + 0x0F);
			tooltipAndButtonProc1ReturnAddress = address + 0x14;

			Injector::MakeJMP(address, tooltipAndButtonProc1V137, true);
			return false;
		}
		return true;
	}

	bool tooltipAndButtonProc2Injector() {
		// mov     r9d, r14d
		BytePattern::temp_instance().find_pattern("45 8B CE 41 0F B6 04 01");
		if (BytePattern::temp_instance().has_size(1, "char fetch in processing loop 1")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			tooltipAndButtonProc2ReturnAddress = address + 0xF;

			Injector::MakeJMP(address, tooltipAndButtonProc2V137, true);
			return false;
		}
		return true;
	}

	bool tooltipAndButtonProc3Injector() {
		// mov     ecx, r15d
		BytePattern::temp_instance().find_pattern("41 8B CF F3 44 0F 10 9A 48 08 00 00");
		if (BytePattern::temp_instance().has_size(1, "char fetch in processing loop 2")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			tooltipAndButtonProc3ReturnAddress = address + 0x18;

			Injector::MakeJMP(address, tooltipAndButtonProc3V137, true);
			return false;
		}
		return true;
	}

	bool tooltipAndButtonProc4Injector() {
		// cmp     word ptr [r11+6], 0
		BytePattern::temp_instance().find_pattern("66 41 83 7B 06 00 0F 85 E0 03");
		if (BytePattern::temp_instance().has_size(1, "line break in processing loop 1")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			tooltipAndButtonProc4ReturnAddress1 = Injector::GetBranchDestination(address + 0x6);
			tooltipAndButtonProc4ReturnAddress2 = Injector::GetBranchDestination(address + 0x10);
			tooltipAndButtonProc4ReturnAddress3 = address + 0x42;

			Injector::MakeJMP(address, tooltipAndButtonProc4V137, true);
			return false;
		}
		return true;
	}

	bool tooltipAndButtonProc5Injector() {
		bool failed = false;

		// movaps  xmm8, [rsp+0F8h+var_58]
		BytePattern::temp_instance().find_pattern("44 0F 28 84 24 A0 00 00 00 0F 28 BC 24 B0 00 00 00 48");
		if (BytePattern::temp_instance().has_size(1, "tooltip line break return address 2")) {
			tooltipAndButtonProc5ReturnAddress2 = BytePattern::temp_instance().get_first().address();
		}
		else {
			failed = true;
		}

		// movzx   edx, byte ptr [rbx+rbp]
		BytePattern::temp_instance().find_pattern("0F B6 14 2B 49 8D 8F 20 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "tooltip line break")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			tooltipAndButtonProc5ReturnAddress1 = address + 0x12;

			Injector::MakeJMP(address, tooltipAndButtonProc5V137, true);
		}
		else {
			failed = true;
		}

		return failed;
	}

	bool tooltipAndButtonProc6Injector() {
		// nbsp
		BytePattern::temp_instance().find_pattern("A7 52 2D 20 00 00 00 00");
		if (BytePattern::temp_instance().has_size(1, "convert space to non-breaking space")) {
			Injector::WriteMemory<uint8_t>(BytePattern::temp_instance().get_first().address() + 3, 0xA0, true);
			return false;
		}
		return true;
	}

	bool tooltipAndButtonProc7Injector() {
		// inc     r14d
		BytePattern::temp_instance().find_pattern("41 FF C6 44 3B 75 D8");
		if (BytePattern::temp_instance().has_size(1, "counter increment")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			tooltipAndButtonProc7ReturnAddress1 = Injector::GetBranchDestination(address + 0x7);
			tooltipAndButtonProc7ReturnAddress2 = address + 0x14;

			Injector::MakeJMP(address, tooltipAndButtonProc7V137, true);
			return false;
		}
		return true;
	}

	HookResult Init(const RunOptions& options) {
		HookResult result("TooltipAndButton");

		result.add("tooltipAndButtonProc1Injector", tooltipAndButtonProc1Injector());
		result.add("tooltipAndButtonProc2Injector", tooltipAndButtonProc2Injector());
		result.add("tooltipAndButtonProc3Injector", tooltipAndButtonProc3Injector());
		result.add("tooltipAndButtonProc4Injector", tooltipAndButtonProc4Injector());
		result.add("tooltipAndButtonProc5Injector", tooltipAndButtonProc5Injector());
		result.add("tooltipAndButtonProc6Injector", tooltipAndButtonProc6Injector());
		result.add("tooltipAndButtonProc7Injector", tooltipAndButtonProc7Injector());

		return result;
	}
}
