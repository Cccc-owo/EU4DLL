#include "../plugin_64.h"
#include "../escape_tool.h"

namespace FileSave {
	extern "C" {
		void fileSaveProc1();
		void fileSaveProc2V137();
		void fileSaveProc3V137();
		void fileSaveProc5V137();
		void fileSaveProc6V137();
		void fileSaveProc6V137E();
		void fileSaveProc6V137G();
		void fileSaveProc7V137();
		void fileSaveProc9();
		void fileSaveProc10();
		uintptr_t fileSaveProc1ReturnAddress;
		uintptr_t fileSaveProc2ReturnAddress;
		uintptr_t fileSaveProc2CallAddress;
		uintptr_t fileSaveProc3ReturnAddress;
		uintptr_t fileSaveProc3CallAddress;
		uintptr_t fileSaveProc3CallAddress2;
		uintptr_t fileSaveProc5ReturnAddress;
		uintptr_t fileSaveProc5CallAddress;
		uintptr_t fileSaveProc6ReturnAddress;
		uintptr_t fileSaveProc6CallAddress;
		uintptr_t fileSaveProc7ReturnAddress;
		uintptr_t fileSaveProc7CallAddress;
		uintptr_t fileSaveProc9ReturnAddress;
		uintptr_t fileSaveProc9CallAddress;
		uintptr_t fileSaveProc10ReturnAddress;
	}

	DllError fileSaveProc1Injector() {
		DllError e = {};

		// mov     eax, [rcx+10h]
		BytePattern::temp_instance().find_pattern("8B 41 10 85 C0 0F 84 0C 01 00 00");
		if (BytePattern::temp_instance().has_size(1, "ファイル名を安全にしている場所を短絡する")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			fileSaveProc1ReturnAddress = Injector::GetBranchDestination(address + 0x5);

			Injector::MakeJMP(address, fileSaveProc1, true);
		}
		else {
			e.fileSave.unmatchdFileSaveProc1Injector = true;
		}

		return e;
	}

	DllError fileSaveProc2Injector() {
		DllError e = {};

		// cmp     qword ptr [rax+18h], 10h
		BytePattern::temp_instance().find_pattern("48 83 78 18 10 72 03 48 8B 00 4C 89 65 D7");
		if (BytePattern::temp_instance().has_size(3, "ファイル名をUTF-8に変換して保存できるようにする")) {
			uintptr_t address = BytePattern::temp_instance().get(2).address();

			fileSaveProc2CallAddress = (uintptr_t)escapedStrToUtf8;
			fileSaveProc2ReturnAddress = address + 0x1A;

			Injector::MakeJMP(address, fileSaveProc2V137, true);
		}
		else {
			e.fileSave.unmatchdFileSaveProc2Injector = true;
		}

		return e;
	}

	DllError fileSaveProc3Injector() {
		DllError e = {};

		BytePattern::temp_instance().find_pattern("45 33 C0 48 8D 96 40 02 00 00 48 8B CD");
		if (BytePattern::temp_instance().has_size(1, "ダイアログでのセーブエントリのタイトルを表示できるようにする")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			fileSaveProc3CallAddress = (uintptr_t)utf8ToEscapedStr;
			fileSaveProc3CallAddress2 = Injector::GetBranchDestination(address + 0xD);
			fileSaveProc3ReturnAddress = address + 0x12;

			Injector::MakeJMP(address, fileSaveProc3V137, true);
		}
		else {
			e.fileSave.unmatchdFileSaveProc3Injector = true;
		}

		return e;
	}

	// proc4 is no-op for v1.37

	DllError fileSaveProc5Injector() {
		DllError e = {};

		// cmp     qword ptr [rdx+18h], 10h
		BytePattern::temp_instance().find_pattern("48 83 7A 18 10 72 03 48 8B 12 48 8D 8C 24 E0 00 00 00 E8 ? ? ? ? 90 0F");
		if (BytePattern::temp_instance().has_size(1, "ダイアログでのセーブエントリのツールチップを表示できるようにする2")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			fileSaveProc5CallAddress = (uintptr_t)utf8ToEscapedStr2;
			fileSaveProc5ReturnAddress = address + 0x12;

			Injector::MakeJMP(address, fileSaveProc5V137, true);
		}
		else {
			e.fileSave.unmatchdFileSaveProc5Injector = true;
		}

		return e;
	}

	DllError fileSaveProc6Injector() {
		DllError e = {};

		// mov     [rsp+3E0h+var_370], 0C0h
		BytePattern::temp_instance().find_pattern("C7 44 24 70 C0 00 00 00 48 8D 95 A0 00 00 00");
		if (BytePattern::temp_instance().has_size(1, "スタート画面でのコンティニューのツールチップ")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address(0x8);

			fileSaveProc6CallAddress = (uintptr_t)utf8ToEscapedStr2;
			fileSaveProc6ReturnAddress = address + 0x23;

			Injector::MakeJMP(address, fileSaveProc6V137, true);
		}
		else {
			// Try alternative pattern E
			BytePattern::temp_instance().find_pattern("C7 44 24 50 C0 00 00 00 48 8D 95 A0 00 00 00");
			if (BytePattern::temp_instance().has_size(1, "スタート画面でのコンティニューのツールチップ")) {
				uintptr_t address = BytePattern::temp_instance().get_first().address(0x8);

				fileSaveProc6CallAddress = (uintptr_t)utf8ToEscapedStr2;
				fileSaveProc6ReturnAddress = address + 0x23;

				Injector::MakeJMP(address, fileSaveProc6V137E, true);
			}
			else {
				// Try alternative pattern G
				BytePattern::temp_instance().find_pattern("48 8D 4C 24 50 E8 1A E6 17 FF 48 8D 54 24 50 48 8D 4D 80");
				if (BytePattern::temp_instance().has_size(1, "スタート画面でのコンティニューのツールチップ")) {
					uintptr_t address = BytePattern::temp_instance().get_first().address(0xa);

					fileSaveProc6CallAddress = (uintptr_t)utf8ToEscapedStr2;
					fileSaveProc6ReturnAddress = address + 0x9;

					Injector::MakeJMP(address, fileSaveProc6V137G, true);
				}
				else {
					e.fileSave.unmatchdFileSaveProc6Injector = true;
				}
			}
		}

		return e;
	}

	DllError fileSaveProc7Injector() {
		DllError e = {};

		// lea     rcx, [rdi+0C8h]
		BytePattern::temp_instance().find_pattern("48 8D 8F C8 00 00 00 48 8B 01 48 8D 54 24 20 FF 90 80");
		if (BytePattern::temp_instance().has_size(1, "セーブダイアログでのインプットテキストエリア")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			fileSaveProc7CallAddress = (uintptr_t)utf8ToEscapedStr2;
			fileSaveProc7ReturnAddress = address + 0xF;

			Injector::MakeJMP(address, fileSaveProc7V137, true);
		}
		else {
			e.fileSave.unmatchdFileSaveProc7Injector = true;
		}

		return e;
	}

	DllError fileSaveProc8Injector() {
		DllError e = {};

		// nop
		BytePattern::temp_instance().find_pattern("90 48 8D 55 E7 48 8D 4D C7");
		if (BytePattern::temp_instance().has_size(1, "ISSUE-231")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			Injector::MakeNOP(address, 0xE, true);
		}
		else {
			e.fileSave.unmatchdFileSaveProc8Injector = true;
		}

		return e;
	}

	void* fileSaveProc9Call(ParadoxTextObject* p) {
		utf8ToEscapedStrP(p);
		return p;
	}

	DllError fileSaveProc9Injector() {
		DllError e = {};

		// mov     [rbp+57h+arg_18], eax
		BytePattern::temp_instance().find_pattern("89 45 7F 48 89 45 CF 48 89 45 0F");
		if (BytePattern::temp_instance().has_size(2, "ISSUE-258")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address(0x17F);

			fileSaveProc9ReturnAddress = address + 0x15;
			fileSaveProc9CallAddress = (uintptr_t)fileSaveProc9Call;

			Injector::MakeJMP(address, fileSaveProc9, true);
		}
		else {
			e.fileSave.unmatchdFileSaveProc9Injector = true;
		}

		return e;
	}

	DllError fileSaveProc10Injector() {
		DllError e = {};

		// mov     [rbp+30h+var_80], rax
		BytePattern::temp_instance().find_pattern("48 89 45 B0 C7 45 D8 01 00 00 00 48 8D 54 24 50 48 83 7C 24");
		if (BytePattern::temp_instance().has_size(3, "ISSUE-258")) {
			uintptr_t address = BytePattern::temp_instance().get(2).address(0x37);

			fileSaveProc10ReturnAddress = address + 0x16;

			Injector::MakeJMP(address, fileSaveProc10, true);
		}
		else {
			e.fileSave.unmatchdFileSaveProc10Injector = true;
		}

		return e;
	}

	DllError Init(RunOptions options) {
		DllError result = {};

		result |= fileSaveProc1Injector();
		result |= fileSaveProc2Injector();
		result |= fileSaveProc3Injector();
		// proc4 is no-op for v1.37
		result |= fileSaveProc5Injector();
		result |= fileSaveProc6Injector();
		result |= fileSaveProc7Injector();
		result |= fileSaveProc8Injector();
		result |= fileSaveProc9Injector();
		result |= fileSaveProc10Injector();

		return result;
	}
}
