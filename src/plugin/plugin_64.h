#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstring>
#include <vector>
#include <string>
#include <utility>

struct HookResult {
	std::string name;
	std::vector<std::pair<std::string, bool>> injectors;

	explicit HookResult(const char* moduleName) : name(moduleName) {}

	void add(const char* injectorName, bool failed) {
		injectors.emplace_back(injectorName, failed);
	}

	bool hasError() const {
		for (const auto& [n, failed] : injectors)
			if (failed) return true;
		return false;
	}

	std::string print() const {
		std::string result;
		for (const auto& [injName, failed] : injectors)
			result += (failed ? "NG" : "OK") + std::string(":") + injName + "\n";
		return result;
	}
};

struct DllError {
	std::vector<HookResult> modules;

	void merge(const HookResult& result) {
		modules.push_back(result);
	}

	bool hasError() const {
		for (const auto& m : modules)
			if (m.hasError()) return true;
		return false;
	}

	std::string print() const {
		std::string result;
		for (size_t i = 0; i < modules.size(); i++) {
			result += modules[i].print();
			if (i + 1 < modules.size())
				result += "--------------\n";
		}
		return result;
	}
};

// same std::basic_string<char>
typedef struct _ParadoxTextObject {
	union {
		char text[0x10];
		char* p;
	} t;
	UINT64 len;
	UINT64 len2;

	std::string getString() {
		if (len2 >= 0x10) {
			return std::string(t.p);
		}
		else {
			return std::string(t.text);
		}
	}

	void setString(std::string *src) {

		if (len2 >= 0x10) {
			auto hHeap = GetProcessHeap();
			HeapFree(hHeap, 0, t.p);
		}

		len2 = src->capacity();
		len = src->length();

		if (len >= 0x10) {
			len2 = 0xFF;
			auto hHeap = GetProcessHeap();
			auto p = static_cast<char*>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 0xFF));
			if (p == nullptr) return;
			memcpy(p, src->c_str(), len + 1);
			t.p = p;
		}
		else {
			memcpy(t.text, src->c_str(), len + 1);
		}
	}

} ParadoxTextObject;

typedef struct _RunOptions {
	bool test;
	bool reversingWordsBattleOfArea;
	bool autoUtf8Conversion;
	int separateCharacterCodePoint;
	int lineBreakBufferWidth;
} RunOptions;

namespace Version {
	bool DetectVersion();
}

namespace Ini {
	void GetOptionsFromIni(RunOptions *option);
}

namespace Font {
	HookResult Init(RunOptions option);
}

namespace MainText {
	HookResult Init(RunOptions option);
}

namespace TooltipAndButton {
	HookResult Init(RunOptions option);
}

namespace MapView {
	HookResult Init(RunOptions option);
}

namespace MapAdjustment {
	HookResult Init(RunOptions option);
}

namespace MapJustify {
	HookResult Init(RunOptions option);
}

namespace EventDialog {
	HookResult Init(RunOptions option);
}

namespace MapPopup {
	HookResult Init(RunOptions option);
}

namespace ListFieldAdjustment {
	HookResult Init(RunOptions option);
}

namespace Validator {
	bool Validate(DllError dllError, RunOptions options);
	bool ValidateVersion();
}

namespace FileSave {
	HookResult Init(RunOptions option);
}

namespace Date {
	HookResult Init(RunOptions option);
}

namespace MapNudgeView {
	HookResult Init(RunOptions option);
}

namespace Ime {
	HookResult Init(RunOptions option);
}

namespace Input {
	HookResult Init(RunOptions option);
}

namespace Localization {
	HookResult Init(RunOptions option);
}

namespace CBitmapFont {
	HookResult Init(RunOptions option);
}

namespace FileRead {
	void Init(RunOptions option);
}
