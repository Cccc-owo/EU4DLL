#include "../plugin_64.h"

namespace Localization {
	extern "C" {
		void localizationProc2V137();
		void localizationProc5V137();
		void localizationProc6V137();
		void localizationProc8V137();

		uintptr_t localizationProc2CallAddress;
		uintptr_t localizationProc2ReturnAddress;
		uintptr_t localizationProc5CallAddress;
		uintptr_t localizationProc5ReturnAddress;
		uintptr_t localizationProc6ReturnAddress;
		uintptr_t localizationProc6CallAddress;
		uintptr_t localizationProc8CallAddress;
		uintptr_t localizationProc8ReturnAddress;

		uintptr_t year;
		uintptr_t month;
		uintptr_t day;
	}

	// Battle of area reversal
	void* localizationProc2Call(ParadoxTextObject* p, const char* src, const size_t size) {
		auto text = std::string(src).substr(1) + p->getString();
		p->setString(&text);
		return p;
	}

	// Name reversal (family name first)
	void* localizationProc5Call(ParadoxTextObject* p1, ParadoxTextObject *p2) {
		auto first = p1->getString();
		auto second = p2->getString();

		if (second.length() >= 3 && second.at(1) == (char)0xBF) {
			auto text = second.substr(2) + first;
			p1->setString(&text);
		}
		else {
			auto text = first + second;
			p1->setString(&text);
		}

		return p1;
	}

	// M, Y -> Y年M
	void* localizationProc6Call(ParadoxTextObject* p1, ParadoxTextObject* p2, ParadoxTextObject* p3) {
		auto text = p3->getString() + std::string("\x0F") + p2->getString().substr(0, p2->len - 2);

		p1->len = 0;
		p1->len2 = 0;
		p1->setString(&text);

		return p1;
	}

	// M Y -> Y年M
	void* localizationProc8Call(ParadoxTextObject* p1, ParadoxTextObject* p2, ParadoxTextObject* p3) {
		auto text = p3->getString() + std::string("\x0F") + p2->getString().substr(0, p2->len - 1);

		p1->len = 0;
		p1->len2 = 0;
		p1->setString(&text);

		return p1;
	}

	DllError localizationProc2Injector(RunOptions options) {
		DllError e = {};

		if (!options.reversingWordsBattleOfArea) return e;

		// movdqu  xmmword ptr [rbp+Size], xmm0
		BytePattern::temp_instance().find_pattern("F3 0F 7F 45 E8 ? ? ? ? 49 C7 C0 FF FF FF FF 0F 1F 44 00 00");
		if (BytePattern::temp_instance().has_size(1, "Battle of areaを逆転させる")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address(0x48);

			localizationProc2ReturnAddress = address + 0x1A;
			localizationProc2CallAddress = (uintptr_t)localizationProc2Call;

			Injector::MakeJMP(address, localizationProc2V137, true);
		}
		else {
			e.localization.unmatchdLocalizationProc2Injector = true;
		}

		return e;
	}

	DllError localizationProc5Injector() {
		DllError e = {};

		// mov     rdx, [rdi+68h]
		BytePattern::temp_instance().find_pattern("48 8B 57 68 48 83 C2 08 48 89 75 B8 48 C7 45 D0 0F 00 00 00");
		if (BytePattern::temp_instance().has_size(1, "nameを逆転させる")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address(0x40);

			localizationProc5ReturnAddress = address + 0x1A;
			localizationProc5CallAddress = (uintptr_t)localizationProc5Call;

			Injector::MakeJMP(address, localizationProc5V137, true);
		}
		else {
			e.localization.unmatchdLocalizationProc5Injector = true;
		}

		return e;
	}

	DllError localizationProc6Injector() {
		DllError e = {};

		// lea     r8, [rbp+57h+var_78]
		BytePattern::temp_instance().find_pattern("4C 8D 45 DF 48 8D 55 BF 48 8D 4D 1F");
		if (BytePattern::temp_instance().has_size(3, "M, Y → Y年M")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			localizationProc6ReturnAddress = address + 0x11;
			localizationProc6CallAddress = (uintptr_t)localizationProc6Call;

			Injector::MakeJMP(address, localizationProc6V137, true);
		}
		else {
			e.localization.unmatchdLocalizationProc6Injector = true;
		}

		return e;
	}

	DllError localizationProc8Injector() {
		DllError e = {};

		// nop
		BytePattern::temp_instance().find_pattern("90 4C 8D 44 24 40 48 8B D0 48 8B CF E8 ? ? ? ? 90");
		if (BytePattern::temp_instance().has_size(2, "M Y → Y年M")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			localizationProc8ReturnAddress = address + 0x11;
			localizationProc8CallAddress = (uintptr_t)localizationProc8Call;

			Injector::MakeJMP(address, localizationProc8V137, true);
		}
		else {
			e.localization.unmatchdLocalizationProc8Injector = true;
		}

		return e;
	}

	DllError localizationProc10Injector() {
		DllError e = {};

		BytePattern::temp_instance().find_pattern("A3 6E 6F 20 00 00"); // £no
		if (BytePattern::temp_instance().has_size(3, "space")) {
			uintptr_t address = BytePattern::temp_instance().get(2).address();
			Injector::WriteMemory<uint8_t>(address + 0, 0xA3, true);
			Injector::WriteMemory<uint8_t>(address + 1, 0x6E, true);
			Injector::WriteMemory<uint8_t>(address + 2, 0x6F, true);
			Injector::WriteMemory<uint8_t>(address + 3, 0x20, true);
			Injector::WriteMemory<uint8_t>(address + 4, 0xA0, true);
		}
		else {
			e.localization.unmatchdLocalizationProc9Injector = true;
		}

		BytePattern::temp_instance().find_pattern("A3 79 65 73 20 00 00"); // £yes
		if (BytePattern::temp_instance().has_size(1, "space")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();
			Injector::WriteMemory<uint8_t>(address + 0, 0xA3, true);
			Injector::WriteMemory<uint8_t>(address + 1, 0x79, true);
			Injector::WriteMemory<uint8_t>(address + 2, 0x65, true);
			Injector::WriteMemory<uint8_t>(address + 3, 0x73, true);
			Injector::WriteMemory<uint8_t>(address + 4, 0x20, true);
			Injector::WriteMemory<uint8_t>(address + 5, 0xA0, true);
		}
		else {
			e.localization.unmatchdLocalizationProc9Injector = true;
		}

		return e;
	}

	ParadoxTextObject _year;
	ParadoxTextObject _month;
	ParadoxTextObject _day;

	DllError Init(RunOptions options) {
		DllError result = {};

		_day.t.text[0] = 0xE;
		_day.t.text[1] = '\0';
		_day.len = 1;
		_day.len2 = 0xF;

		_year.t.text[0] = 0xF;
		_year.t.text[1] = '\0';
		_year.len = 1;
		_year.len2 = 0xF;

		_month.t.text[0] = 7;
		_month.t.text[1] = '\0';
		_month.len = 1;
		_month.len2 = 0xF;

		year = (uintptr_t)&_year;
		month = (uintptr_t)&_month;
		day = (uintptr_t)&_day;

		// proc1: no-op for v1.37
		// Battle of area reversal
		result |= localizationProc2Injector(options);
		// proc3: no-op for v1.37
		// proc4: no-op for v1.37
		// Name reversal
		result |= localizationProc5Injector();
		// M, Y -> Y年M
		result |= localizationProc6Injector();
		// proc7: no-op for v1.37
		// M Y -> Y年M
		result |= localizationProc8Injector();
		// proc9: no-op for v1.37
		// £no/£yes nbsp
		result |= localizationProc10Injector();

		return result;
	}
}
