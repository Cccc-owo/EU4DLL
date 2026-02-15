add_library(plugin64 SHARED
    dllmain.cpp
    byte_pattern.cpp
    escape_tool.cpp
    options.cpp
    version_detect.cpp
    validator.cpp

    hooks/font.cpp
    hooks/main_text.cpp
    hooks/tooltip_and_button.cpp
    hooks/map_view.cpp
    hooks/map_adjustment.cpp
    hooks/map_justify.cpp
    hooks/map_nudge_view.cpp
    hooks/map_popup.cpp
    hooks/event_dialog.cpp
    hooks/list_field_adjustment.cpp
    hooks/file_save.cpp
    hooks/date.cpp
    hooks/ime.cpp
    hooks/input.cpp
    hooks/localization.cpp
    hooks/c_bitmap_font.cpp
    hooks/file_read.cpp
    hooks/steam_rich_presence.cpp

    hooks/font_asm.S
    hooks/main_text_asm.S
    hooks/map_view_asm.S
    hooks/map_adjustment_asm.S
    hooks/map_justify_asm.S
    hooks/map_nudge_view_asm.S
    hooks/map_popup_asm.S
    hooks/tooltip_and_button_asm.S
    hooks/event_dialog_asm.S
    hooks/list_field_adjustment_asm.S
    hooks/file_save_asm.S
    hooks/date_asm.S
    hooks/ime_asm.S
    hooks/input_asm.S
    hooks/localization_asm.S
    hooks/c_bitmap_font_asm.S
)

set_target_properties(plugin64 PROPERTIES
    OUTPUT_NAME "plugin64"
    PREFIX ""
    SUFFIX ".dll"
)

target_include_directories(plugin64 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(plugin64 PRIVATE
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

# Enable assembly with Intel syntax for GAS
set_source_files_properties(
    hooks/font_asm.S
    hooks/main_text_asm.S
    hooks/map_view_asm.S
    hooks/map_adjustment_asm.S
    hooks/map_justify_asm.S
    hooks/map_nudge_view_asm.S
    hooks/map_popup_asm.S
    hooks/tooltip_and_button_asm.S
    hooks/event_dialog_asm.S
    hooks/list_field_adjustment_asm.S
    hooks/file_save_asm.S
    hooks/date_asm.S
    hooks/ime_asm.S
    hooks/input_asm.S
    hooks/localization_asm.S
    hooks/c_bitmap_font_asm.S
    PROPERTIES LANGUAGE ASM
)

target_link_libraries(plugin64 PRIVATE
    kernel32
    user32
    shell32
    ole32
)

if(MINGW)
    target_compile_options(plugin64 PRIVATE -static)
    target_link_options(plugin64 PRIVATE -static -static-libgcc -static-libstdc++)
endif()
