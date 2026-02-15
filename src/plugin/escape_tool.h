#pragma once

#include "plugin_64.h"

char* utf8ToEscapedStr(char* from);
ParadoxTextObject* utf8ToEscapedStr2(ParadoxTextObject* from);
char* escapedStrToUtf8(ParadoxTextObject* from);
char* utf8ToEscapedStr3(char* from);
errno_t convertWideTextToEscapedText(const wchar_t* from, char** to);
errno_t convertTextToWideText(const char* from, wchar_t** to);
void utf8ToEscapedStrP(ParadoxTextObject* src);
