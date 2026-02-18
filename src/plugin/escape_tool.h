#pragma once

#include "plugin_64.h"

char*              utf8ToEscapedStr(char* from);
ParadoxTextObject* utf8ToEscapedStr2(ParadoxTextObject* from);
char*              escapedStrToUtf8(ParadoxTextObject* from);
char*              utf8ToEscapedStr3(char* from);
void               utf8ToEscapedStrP(ParadoxTextObject* src);

wchar_t      CP1252ToUCS2(unsigned char cp);
std::string  convertWideTextToEscapedText(const wchar_t* from, bool forUtf8 = false);
std::wstring convertTextToWideText(const char* from);
std::wstring convertEscapedTextToWideText(const std::string& from);
std::string  convertWideTextToUtf8(const std::wstring& from);
