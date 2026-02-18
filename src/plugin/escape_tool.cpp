#include "escape_tool.h"
#include <memory>

inline wchar_t UCS2ToCP1252(int cp) {
    wchar_t result = cp;
    switch (cp) {
        case 0x20AC: result = 0x80; break;
        case 0x201A: result = 0x82; break;
        case 0x0192: result = 0x83; break;
        case 0x201E: result = 0x84; break;
        case 0x2026: result = 0x85; break;
        case 0x2020: result = 0x86; break;
        case 0x2021: result = 0x87; break;
        case 0x02C6: result = 0x88; break;
        case 0x2030: result = 0x89; break;
        case 0x0160: result = 0x8A; break;
        case 0x2039: result = 0x8B; break;
        case 0x0152: result = 0x8C; break;
        case 0x017D: result = 0x8E; break;
        case 0x2018: result = 0x91; break;
        case 0x2019: result = 0x92; break;
        case 0x201C: result = 0x93; break;
        case 0x201D: result = 0x94; break;
        case 0x2022: result = 0x95; break;
        case 0x2013: result = 0x96; break;
        case 0x2014: result = 0x97; break;
        case 0x02DC: result = 0x98; break;
        case 0x2122: result = 0x99; break;
        case 0x0161: result = 0x9A; break;
        case 0x203A: result = 0x9B; break;
        case 0x0153: result = 0x9C; break;
        case 0x017E: result = 0x9E; break;
        case 0x0178: result = 0x9F; break;
    }

    return result;
}

wchar_t CP1252ToUCS2(unsigned char cp) {
    wchar_t result = cp;
    switch (cp) {
        case 0x80: result = 0x20AC; break;
        case 0x82: result = 0x201A; break;
        case 0x83: result = 0x0192; break;
        case 0x84: result = 0x201E; break;
        case 0x85: result = 0x2026; break;
        case 0x86: result = 0x2020; break;
        case 0x87: result = 0x2021; break;
        case 0x88: result = 0x02C6; break;
        case 0x89: result = 0x2030; break;
        case 0x8A: result = 0x0160; break;
        case 0x8B: result = 0x2039; break;
        case 0x8C: result = 0x0152; break;
        case 0x8E: result = 0x017D; break;
        case 0x91: result = 0x2018; break;
        case 0x92: result = 0x2019; break;
        case 0x93: result = 0x201C; break;
        case 0x94: result = 0x201D; break;
        case 0x95: result = 0x2022; break;
        case 0x96: result = 0x2013; break;
        case 0x97: result = 0x2014; break;
        case 0x98: result = 0x02DC; break;
        case 0x99: result = 0x2122; break;
        case 0x9A: result = 0x0161; break;
        case 0x9B: result = 0x203A; break;
        case 0x9C: result = 0x0153; break;
        case 0x9E: result = 0x017E; break;
        case 0x9F: result = 0x0178; break;
        default: break;
    }

    return result;
}

std::string convertWideTextToEscapedText(const wchar_t* from, bool forUtf8) {
    if (!from)
        return {};

    size_t      size = wcslen(from);
    std::string result;
    result.reserve(size * 3);

    for (unsigned int fromIndex = 0; fromIndex < size; fromIndex++) {
        wchar_t cp = from[fromIndex];

        if (!forUtf8) {
            wchar_t mapped = UCS2ToCP1252(cp);
            if (mapped != cp) {
                result += static_cast<char>(static_cast<BYTE>(mapped));
                continue;
            }
        }

        if (cp > 0x100 && cp < 0xA00) {
            cp = cp + 0xE000;
        }

        BYTE high = (cp >> 8) & 0x000000FF;
        BYTE low  = cp & 0x000000FF;

        BYTE escapeChr = 0x10;

        if (high == 0) {
            result += static_cast<char>(static_cast<BYTE>(cp));
            continue;
        }

        switch (high) {
            case 0xA4:
            case 0xA3:
            case 0xA7:
            case 0x24:
            case 0x5B:
            case 0x00:
            case 0x5C:
            case 0x20:
            case 0x0D:
            case 0x0A:
            case 0x22:
            case 0x7B:
            case 0x7D:
            case 0x40:
            case 0x80:
            case 0x7E:
            case 0x2F:
            case 0x5F:
            case 0xBD:
            case 0x3B:
            case 0x5D:
            case 0x3D:
            case 0x23:
            case 0x3F:
            case 0x3A:
            case 0x3C:
            case 0x3E:
            case 0x2A:
            case 0x7C: escapeChr += 2; break;
            default: break;
        }

        switch (low) {
            case 0xA4:
            case 0xA3:
            case 0xA7:
            case 0x24:
            case 0x5B:
            case 0x00:
            case 0x5C:
            case 0x20:
            case 0x0D:
            case 0x0A:
            case 0x22:
            case 0x7B:
            case 0x7D:
            case 0x40:
            case 0x80:
            case 0x7E:
            case 0x2F:
            case 0x5F:
            case 0xBD:
            case 0x3B:
            case 0x5D:
            case 0x3D:
            case 0x23:
            case 0x3F:
            case 0x3A:
            case 0x3C:
            case 0x3E:
            case 0x2A:
            case 0x7C: escapeChr++; break;
            default: break;
        }

        switch (escapeChr) {
            case 0x11: low += 14; break;
            case 0x12: high -= 9; break;
            case 0x13:
                low += 14;
                high -= 9;
                break;
            case 0x10:
            default: break;
        }

        result += static_cast<char>(escapeChr);
        result += static_cast<char>(low);
        result += static_cast<char>(high);
    }

    return result;
}

std::wstring convertEscapedTextToWideText(const std::string& from) {
    std::wstring result;

    for (unsigned int fromIndex = 0; fromIndex < from.length();) {
        BYTE   cp   = (BYTE)from[fromIndex++];
        BYTE   low  = 0;
        BYTE   high = 0;
        UINT32 sp   = 0;

        switch (cp) {
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
                if (fromIndex + 2 > from.length())
                    return result;
                low  = (BYTE)from[fromIndex++];
                high = (BYTE)from[fromIndex++];

                sp = ((high) << 8) + low;

                switch (cp) {
                    case 0x10: break;
                    case 0x11: sp -= 0xE; break;
                    case 0x12: sp += 0x900; break;
                    case 0x13: sp += 0x8F2; break;
                    default: break;
                }
                break;
            default: sp = CP1252ToUCS2(cp); break;
        }

        if (sp > 0xFFFF || (sp < 0x98F && sp > 0x100)) {
            sp = 0x2026;
        }

        result.append(1, static_cast<wchar_t>(sp));
    }

    return result;
}

std::wstring convertTextToWideText(const char* from) {
    if (!from)
        return {};

    int wideTextSize = MultiByteToWideChar(CP_UTF8, 0, from, -1, NULL, 0);
    if (wideTextSize == 0)
        return {};

    std::wstring result(wideTextSize, L'\0');
    int          err = MultiByteToWideChar(CP_UTF8, 0, from, -1, result.data(), wideTextSize);
    if (err == 0)
        return {};

    // Remove null terminator (MultiByteToWideChar with -1 includes it)
    if (!result.empty() && result.back() == L'\0')
        result.pop_back();

    return result;
}

std::string convertWideTextToUtf8(const std::wstring& from) {
    int textSize = WideCharToMultiByte(CP_UTF8, 0, from.c_str(), -1, NULL, 0, NULL, NULL);
    if (textSize == 0)
        return {};

    std::string result(textSize, '\0');
    int         err =
        WideCharToMultiByte(CP_UTF8, 0, from.c_str(), -1, result.data(), textSize, NULL, NULL);
    if (err == 0)
        return {};

    // Remove null terminator
    if (!result.empty() && result.back() == '\0')
        result.pop_back();

    return result;
}

static thread_local std::unique_ptr<ParadoxTextObject> tmpParadoxTextObject;
char*                                                  utf8ToEscapedStr(char* from) {

    if (tmpParadoxTextObject) {
        if (tmpParadoxTextObject->len >= 0x10) {
            HeapFree(GetProcessHeap(), 0, tmpParadoxTextObject->t.p);
        }
    }

    tmpParadoxTextObject = std::make_unique<ParadoxTextObject>();

    char* src = nullptr;

    if (*(from + 0x10) >= 0x10) {
        src = reinterpret_cast<char*>(*reinterpret_cast<uintptr_t*>(from));
    } else {
        src = from;
    }

    std::wstring wide    = convertTextToWideText(src);
    std::string  escaped = convertWideTextToEscapedText(wide.c_str());

    UINT64 len                 = escaped.length();
    tmpParadoxTextObject->len  = len;
    tmpParadoxTextObject->len2 = len;

    if (len >= 0x10) {
        auto  hHeap = GetProcessHeap();
        char* buf   = static_cast<char*>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY, len + 1));
        if (buf) {
            memcpy(buf, escaped.c_str(), len + 1);
            tmpParadoxTextObject->t.p = buf;
        }
    } else {
        memcpy(tmpParadoxTextObject->t.text, escaped.c_str(), len + 1);
    }

    return reinterpret_cast<char*>(tmpParadoxTextObject.get());
}

void utf8ToEscapedStrP(ParadoxTextObject* src) {

    std::wstring wide    = convertTextToWideText(src->getString().c_str());
    std::string  escaped = convertWideTextToEscapedText(wide.c_str());

    src->setString(escaped);
}

static thread_local std::unique_ptr<ParadoxTextObject> tmpZV2;
ParadoxTextObject*                                     utf8ToEscapedStr2(ParadoxTextObject* from) {

    if (tmpZV2) {
        if (tmpZV2->len >= 0x10) {
            HeapFree(GetProcessHeap(), 0, tmpZV2->t.p);
        }
    }
    tmpZV2 = std::make_unique<ParadoxTextObject>();

    char* src = nullptr;

    if (from->len >= 0x10) {
        src = from->t.p;
    } else {
        src = from->t.text;
    }

    std::wstring wide    = convertTextToWideText(src);
    std::string  escaped = convertWideTextToEscapedText(wide.c_str());

    UINT64 len  = escaped.length();
    tmpZV2->len = len;

    if (len >= 0x10) {
        auto  hHeap = GetProcessHeap();
        char* buf   = static_cast<char*>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY, len + 1));
        if (buf) {
            memcpy(buf, escaped.c_str(), len + 1);
            tmpZV2->t.p = buf;
        }
        tmpZV2->len2 = 0x1F;
    } else {
        memcpy(tmpZV2->t.text, escaped.c_str(), len + 1);
    }

    return tmpZV2.get();
}

char* escapedStrToUtf8(ParadoxTextObject* from) {

    std::string  src  = from->getString();
    std::wstring wide = convertEscapedTextToWideText(src);
    std::string  utf8 = convertWideTextToUtf8(wide);

    from->setString(utf8);

    return reinterpret_cast<char*>(from);
}

static thread_local std::string utf8ToEscapedStr3buffer;
char*                           utf8ToEscapedStr3(char* from) {
    std::wstring wide       = convertTextToWideText(from);
    utf8ToEscapedStr3buffer = convertWideTextToEscapedText(wide.c_str());

    return utf8ToEscapedStr3buffer.data();
}
