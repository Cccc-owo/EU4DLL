#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace EventDialog {
	extern "C" {
		void eventDialogProc1V137();
		void eventDialogProc2V137();
		void eventDialogProc3V137();
		uintptr_t eventDialogProc1ReturnAddress;
		uintptr_t eventDialogProc2ReturnAddress1;
		uintptr_t eventDialogProc2ReturnAddress2;
		uintptr_t eventDialogProc3ReturnAddress;
	}

	bool eventDialog1Injector() {
		// movzx   eax, byte ptr [rdx+rax]
		BytePattern::temp_instance().find_pattern("0F B6 04 10 4D 8B 9C C5 20 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "char fetch")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			eventDialogProc1ReturnAddress = address + 0x18;

			Injector::MakeJMP(address, eventDialogProc1V137, true);
			return false;
		}
		return true;
	}

	bool eventDialog2Injector() {
		bool failed = false;

		// mov     rax, [rbp+1060h+arg_20]
		BytePattern::temp_instance().find_pattern("48 8B 85 90 10 00 00 8B 00 03 C0");
		if (BytePattern::temp_instance().has_size(1, "branch fix return address 2")) {
			eventDialogProc2ReturnAddress2 = BytePattern::temp_instance().get_first().address();
		}
		else {
			failed = true;
		}

		// cvtdq2ps xmm0, xmm0
		BytePattern::temp_instance().find_pattern("0F 5B C0 F3 0F 59 C1 41 0F 2E C0 7A 53 75 51");
		if (BytePattern::temp_instance().has_size(1, "branch fix")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			eventDialogProc2ReturnAddress1 = address + 0x0F;

			Injector::MakeJMP(address, eventDialogProc2V137, true);
		}
		else {
			failed = true;
		}

		return failed;
	}

	bool eventDialog3Injector() {
		// inc     edi
		BytePattern::temp_instance().find_pattern("FF C7 44 8B 4B 10 41 3B F9 8B 8D 70 10 00 00");
		if (BytePattern::temp_instance().has_size(1, "counter increment")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			eventDialogProc3ReturnAddress = address + 0xF;

			Injector::MakeJMP(address, eventDialogProc3V137, true);
			return false;
		}
		return true;
	}

	HookResult Init(RunOptions options) {
		HookResult result("EventDialog");

		result.add("eventDialog1Injector", eventDialog1Injector());
		result.add("eventDialog2Injector", eventDialog2Injector());
		result.add("eventDialog3Injector", eventDialog3Injector());

		return result;
	}
}
