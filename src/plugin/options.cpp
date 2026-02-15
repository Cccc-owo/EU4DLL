#include "plugin64.h"
#include "plugin_64.h"

using namespace std;

namespace Ini {
	std::wstring constructIniPath() {
		wchar_t exe_path[1024];
		GetModuleFileName(NULL, exe_path, 1024);

		std::filesystem::path exePath{exe_path};
		auto pluginsDir = exePath.parent_path() / L"plugins";
		auto iniPath = pluginsDir / L"plugin.ini";

		return iniPath.wstring();
	}

	void testMode(const std::wstring& ini_path, RunOptions *options) {
		wchar_t buf[64] = {0};

		GetPrivateProfileStringW(
			L"options",
			L"ERROR_TEST",
			L"no",
			buf,
			64,
			ini_path.c_str()
		);
		options->test = lstrcmpW(buf, L"yes") == 0;
	}

	void separateCharacter(const std::wstring& ini_path, RunOptions* options) {
		wchar_t buf[64] = { 0 };

		GetPrivateProfileStringW(
			L"options",
			L"SEPARATE_CHARACTER_CODE_POINT",
			L"32",
			buf,
			64,
			ini_path.c_str()
		);
		options->separateCharacterCodePoint = _wtoi(buf);
	}

	void lineBreakBufferWidth(const std::wstring& ini_path, RunOptions* options) {
		wchar_t buf[64] = { 0 };

		GetPrivateProfileStringW(
			L"options",
			L"LINE_BREAK_BUFFER_WIDTH",
			L"5",
			buf,
			64,
			ini_path.c_str()
		);
		options->lineBreakBufferWidth = _wtoi(buf);
	}

	void reversingWordsBattleOfArea(const std::wstring& ini_path, RunOptions* options) {
		wchar_t buf[64] = { 0 };

		GetPrivateProfileStringW(
			L"options",
			L"REVERSING_WORDS_BATTLE_OF_AREA",
			L"yes",
			buf,
			64,
			ini_path.c_str()
		);
		options->reversingWordsBattleOfArea = lstrcmpW(buf, L"yes") == 0;
	}

	void autoUtf8Conversion(const std::wstring& ini_path, RunOptions* options) {
		wchar_t buf[64] = { 0 };

		GetPrivateProfileStringW(
			L"options",
			L"AUTO_UTF8_CONVERSION",
			L"yes",
			buf,
			64,
			ini_path.c_str()
		);
		options->autoUtf8Conversion = lstrcmpW(buf, L"yes") == 0;
	}

	void GetOptionsFromIni(RunOptions* options) {
		std::wstring ini_path = constructIniPath();

		testMode(ini_path, options);
		separateCharacter(ini_path, options);
		reversingWordsBattleOfArea(ini_path, options);
		lineBreakBufferWidth(ini_path, options);
		autoUtf8Conversion(ini_path, options);
	}
}
