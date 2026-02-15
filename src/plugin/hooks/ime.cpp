#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace Ime {
	extern "C" {
		void imeProc1V137();
		void imeProc2();
		void imeProc3V137();
		uintptr_t imeProc1ReturnAddress1;
		uintptr_t imeProc1ReturnAddress2;
		uintptr_t imeProc1CallAddress;
		uintptr_t imeProc2CallAddress;
		uintptr_t imeProc2ReturnAddress1;
		uintptr_t imeProc2ReturnAddress2;
		uintptr_t imeProc2ReturnAddress3;
		uintptr_t rectAddress;
		uintptr_t imeProc3ReturnAddress;
		uintptr_t imeProc3CallAddress1;
		uintptr_t imeProc3CallAddress2;
		uintptr_t imeProc3CallAddress3;
		uintptr_t imeProc3CallAddress4;
		uintptr_t imeProc3CallAddress5;
	}

	typedef struct SDL_Rect
	{
		int x, y;
		int w, h;
	} SDL_Rect;

	SDL_Rect rect = { 0,0,0,0 };

	bool imeProc1Injector() {
		// mov     edx, edi
		BytePattern::temp_instance().find_pattern("8B D7 49 8B CC E8 ? ? ? ? 85 C0 0F 85 5B");
		if (BytePattern::temp_instance().has_size(1, "SDL_windowsevents.c fix")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			imeProc1CallAddress = Injector::GetBranchDestination(address + 0x5);

			imeProc1ReturnAddress1 = address + 0x12;

			imeProc1ReturnAddress2 = address - 0x3A;

			Injector::MakeJMP(address, imeProc1V137, true);
			return false;
		}
		return true;
	}

	bool imeProc2Injector() {
		bool failed = false;

		HMODULE handle = NULL;

		rectAddress = (uintptr_t)&rect;

		// SDL_SetTextInputRect
		handle = GetModuleHandle(NULL);
		imeProc2CallAddress = (uintptr_t)GetProcAddress(handle, "SDL_SetTextInputRect");

		if (imeProc2CallAddress == 0) {
			failed = true;
		}

		// WM_IME_STARTCOMPOSITION
		BytePattern::temp_instance().find_pattern("81 EA BC 00 00 00 0F 84 C3 02 00 00");
		if (BytePattern::temp_instance().has_size(1, "SDL_windowskeyboard.c fix")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			imeProc2ReturnAddress1 = Injector::GetBranchDestination(address + 0x6);

			imeProc2ReturnAddress2 = address + 0xF;

			Injector::MakeJMP(address, imeProc2, true);
		}
		else {
			failed = true;
		}

		// WM_IME_SETCONTEXT: NOP *lParam = 0
		// mov     [r9], r13
		BytePattern::temp_instance().find_pattern("4D 89 29 48 8B 5C 24 50");
		if (BytePattern::temp_instance().has_size(1, "SDL_windowskeyboard.c fix")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();
			Injector::MakeNOP(address, 3, true);
		}
		else {
			failed = true;
		}

		// WM_IME_COMPOSITION: skip IME_GetCompositionString and IME_SendInputEvent
		// mov     r8d, 800h
		BytePattern::temp_instance().find_pattern("41 B8 00 08 00 00 48 89 7C 24 58");
		if (BytePattern::temp_instance().has_size(1, "SDL_windowskeyboard.c fix")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address(-2);
			Injector::MakeJMP(address, (void*)(address + 0x9D), true);
		}
		else {
			failed = true;
		}

		return failed;
	}

	bool imeProc3Injector() {
		// mov     rcx, [rbp+0C0h+hRawInput]
		BytePattern::temp_instance().find_pattern("48 8B 8D F8 00 00 00 48 8B D6 E8 ? ? ? ? 33");
		if (BytePattern::temp_instance().has_size(2, "SDL_windowsevents.c fix")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			imeProc3CallAddress1 = Injector::GetBranchDestination(address + 0xA);
			imeProc3CallAddress2 = Injector::GetBranchDestination(address + 0x13);
			imeProc3CallAddress3 = Injector::GetBranchDestination(address + 0x36);
			imeProc3CallAddress4 = Injector::GetBranchDestination(address + 0x50);
			imeProc3CallAddress5 = Injector::GetBranchDestination(address + 0x65);

			imeProc3ReturnAddress = address + 0x6A;

			Injector::MakeJMP(address, imeProc3V137, true);
			return false;
		}
		return true;
	}

	HookResult Init(RunOptions options) {
		HookResult result("Ime");

		result.add("imeProc1Injector", imeProc1Injector());
		result.add("imeProc2Injector", imeProc2Injector());
		result.add("imeProc3Injector", imeProc3Injector());

		return result;
	}
}
