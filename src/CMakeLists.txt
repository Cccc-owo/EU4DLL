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

# Package target: cmake --build build --target package
set(PACKAGE_DIR ${CMAKE_BINARY_DIR}/package)
add_custom_target(package
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}/plugins
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:version> ${PACKAGE_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:plugin64> ${PACKAGE_DIR}/plugins/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resource/plugin.ini ${PACKAGE_DIR}/plugins/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../README.md ${PACKAGE_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/../README_zh-CN.md ${PACKAGE_DIR}/
    COMMAND ${CMAKE_COMMAND}
        -DPACKAGE_DIR=${PACKAGE_DIR}
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake
    DEPENDS version plugin64
    COMMENT "Packaging into ${PACKAGE_DIR}"
)
