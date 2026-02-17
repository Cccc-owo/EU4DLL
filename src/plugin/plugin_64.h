#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>

struct HookResult {
    std::string                               name;
    std::vector<std::pair<std::string, bool>> injectors;

    explicit HookResult(const char* moduleName) : name(moduleName) {}

    void add(const char* injectorName, bool failed) {
        injectors.emplace_back(injectorName, failed);
    }

    bool hasError() const {
        for (const auto& [n, failed] : injectors)
            if (failed)
                return true;
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
            if (m.hasError())
                return true;
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

// same layout as std::basic_string<char>
struct ParadoxTextObject {
    union {
        char  text[0x10];
        char* p;
    } t;
    UINT64 len;
    UINT64 len2;

    std::string getString() {
        if (len2 >= 0x10) {
            return std::string(t.p);
        } else {
            return std::string(t.text);
        }
    }

    void setString(const std::string& src) {

        if (len2 >= 0x10) {
            auto hHeap = GetProcessHeap();
            HeapFree(hHeap, 0, t.p);
        }

        len = src.length();

        if (len >= 0x10) {
            len2       = 0xFF;
            auto hHeap = GetProcessHeap();
            auto p     = static_cast<char*>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 0xFF));
            if (p == nullptr)
                return;
            memcpy(p, src.c_str(), len + 1);
            t.p = p;
        } else {
            len2 = 0xF;
            memcpy(t.text, src.c_str(), len + 1);
        }
    }
};

struct RunOptions {
    bool test;
    bool reversingWordsBattleOfArea;
    bool autoUtf8Conversion;
    int  separateCharacterCodePoint;
    int  lineBreakBufferWidth;
    bool steamRichPresence;
};

namespace Version {
    bool DetectVersion();
}

namespace Ini {
    void GetOptionsFromIni(RunOptions* option);
}

namespace Font {
    HookResult Init(const RunOptions& options);
}

namespace MainText {
    HookResult Init(const RunOptions& options);
}

namespace TooltipAndButton {
    HookResult Init(const RunOptions& options);
}

namespace MapView {
    HookResult Init(const RunOptions& options);
}

namespace MapAdjustment {
    HookResult Init(const RunOptions& options);
}

namespace MapJustify {
    HookResult Init(const RunOptions& options);
}

namespace EventDialog {
    HookResult Init(const RunOptions& options);
}

namespace MapPopup {
    HookResult Init(const RunOptions& options);
}

namespace ListFieldAdjustment {
    HookResult Init(const RunOptions& options);
}

namespace Validator {
    bool Validate(const DllError& dllError, const RunOptions& options);
    bool ValidateVersion();
} // namespace Validator

namespace FileSave {
    HookResult Init(const RunOptions& options);
}

namespace Date {
    HookResult Init(const RunOptions& options);
}

namespace MapNudgeView {
    HookResult Init(const RunOptions& options);
}

namespace Ime {
    HookResult Init(const RunOptions& options);
}

namespace Input {
    HookResult Init(const RunOptions& options);
}

namespace Localization {
    HookResult Init(const RunOptions& options);
}

namespace CBitmapFont {
    HookResult Init(const RunOptions& options);
}

namespace FileRead {
    void Init(const RunOptions& options);
}

namespace SteamRichPresence {
    void Init();
}
