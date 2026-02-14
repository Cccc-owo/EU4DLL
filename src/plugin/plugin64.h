#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <utility>
#include <filesystem>
#include <algorithm>
#include <cassert>
#include <functional>
#include <cstdio>

#include "byte_pattern.h"
#include "injector.h"
