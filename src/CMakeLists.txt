cmake_minimum_required(VERSION 3.20)
project(EU4dll LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-only target (cross-compiled DLLs)
if(NOT WIN32)
    message(FATAL_ERROR "This project produces Windows DLLs. Use a MinGW-w64 cross-compilation toolchain.")
endif()

add_subdirectory(loader)
add_subdirectory(plugin)
