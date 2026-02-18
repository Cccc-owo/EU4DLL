#include "plugin_64.h"
#include <filesystem>

namespace Ini {
    std::wstring constructIniPath() {
        wchar_t exe_path[1024];
        GetModuleFileName(NULL, exe_path, 1024);

        std::filesystem::path exePath{exe_path};
        auto                  pluginsDir = exePath.parent_path() / L"plugins";
        auto                  iniPath    = pluginsDir / L"plugin.ini";

        return iniPath.wstring();
    }

    std::wstring readString(const std::wstring& ini, const wchar_t* key, const wchar_t* def) {
        wchar_t buf[64] = {0};
        GetPrivateProfileStringW(L"options", key, def, buf, 64, ini.c_str());
        return buf;
    }

    bool readBool(const std::wstring& ini, const wchar_t* key, bool def) {
        return readString(ini, key, def ? L"yes" : L"no") == L"yes";
    }

    int readInt(const std::wstring& ini, const wchar_t* key, int def) {
        return _wtoi(readString(ini, key, std::to_wstring(def).c_str()).c_str());
    }

    void GetOptionsFromIni(RunOptions* options) {
        std::wstring ini = constructIniPath();

        options->test                       = readBool(ini, L"ERROR_TEST", false);
        options->separateCharacterCodePoint = readInt(ini, L"SEPARATE_CHARACTER_CODE_POINT", 32);
        options->reversingWordsBattleOfArea =
            readBool(ini, L"REVERSING_WORDS_BATTLE_OF_AREA", true);
        options->lineBreakBufferWidth = readInt(ini, L"LINE_BREAK_BUFFER_WIDTH", 5);
        options->autoUtf8Conversion   = readBool(ini, L"AUTO_UTF8_CONVERSION", true);
        options->steamRichPresence    = readBool(ini, L"STEAM_RICH_PRESENCE", true);

        options->achievementUnlock = readBool(ini, L"ACHIEVEMENT_UNLOCK", false);
    }
} // namespace Ini
