add_library(version SHARED
    dllmain.cpp
)

set_target_properties(version PROPERTIES
    OUTPUT_NAME "version"
    PREFIX ""
    SUFFIX ".dll"
)

# Use .def file for exports
if(MINGW)
    target_link_options(version PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/version_exports.def"
    )
else()
    target_sources(version PRIVATE version_exports.def)
endif()

target_link_libraries(version PRIVATE
    kernel32
    ole32
    shell32
)

if(MINGW)
    target_link_options(version PRIVATE -static -static-libgcc -static-libstdc++)
endif()
