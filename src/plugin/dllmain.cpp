#include "byte_pattern.h"
#include "plugin_64.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {

    if (ulReasonForCall == DLL_PROCESS_ATTACH) {
        BytePattern::StartLog(L"eu4_jps_2");

        DllError e;

        RunOptions options = {};

        // INI settings
        BytePattern::LoggingInfo("[init] Loading INI options...\n");
        Ini::GetOptionsFromIni(&options);
        BytePattern::LoggingInfo(
            "[init] Options: autoUtf8=" + std::to_string(options.autoUtf8Conversion) +
            " steamRP=" + std::to_string(options.steamRichPresence) +
            " achievementUnlock=" + std::to_string(options.achievementUnlock) + "\n");

        // Version check (v1.37 only)
        BytePattern::LoggingInfo("[init] Validating game version...\n");
        if (Validator::ValidateVersion()) {
            BytePattern::LoggingInfo("[init] Version OK\n");

            // UTF-8 auto conversion (IAT hook)
            BytePattern::LoggingInfo("[init] FileRead::Init...\n");
            FileRead::Init(options);
            BytePattern::LoggingInfo("[init] FileRead::Init done\n");

            // Steam rich presence text fix (vtable hook)
            if (options.steamRichPresence) {
                BytePattern::LoggingInfo("[init] SteamRichPresence::Init...\n");
                SteamRichPresence::Init();
                BytePattern::LoggingInfo("[init] SteamRichPresence::Init done (thread launched)\n");
            } else {
                BytePattern::LoggingInfo("[init] SteamRichPresence skipped (disabled)\n");
            }

            // Font loading
            BytePattern::LoggingInfo("[init] Font::Init...\n");
            e.merge(Font::Init(options));

            // UI text display
            BytePattern::LoggingInfo("[init] MainText::Init...\n");
            e.merge(MainText::Init(options));

            // Tooltip and button display
            e.merge(TooltipAndButton::Init(options));

            // Map text display
            e.merge(MapView::Init(options));

            // Map text display (nudge)
            e.merge(MapNudgeView::Init(options));

            // Map text adjustment
            e.merge(MapAdjustment::Init(options));

            // Map text justify
            e.merge(MapJustify::Init(options));

            // Event dialog and map text adjustment
            e.merge(EventDialog::Init(options));

            // Map popup text display
            e.merge(MapPopup::Init(options));

            // List field adjustment
            e.merge(ListFieldAdjustment::Init(options));

            // File save
            e.merge(FileSave::Init(options));

            // Date format
            e.merge(Date::Init(options));

            // IME
            e.merge(Ime::Init(options));

            // Input
            e.merge(Input::Init(options));

            // Localization string reorder
            e.merge(Localization::Init(options));

            // BitmapFont adjustment, scroll adjustment
            e.merge(CBitmapFont::Init(options));

            BytePattern::LoggingInfo("[init] All hooks initialized, validating...\n");
            Validator::Validate(e, options);
        } else {
            BytePattern::LoggingInfo("[init] Version check FAILED\n");
        }

        BytePattern::LoggingInfo("[init] DLL_PROCESS_ATTACH complete\n");
    } else if (ulReasonForCall == DLL_PROCESS_DETACH) {
        BytePattern::ShutdownLog();
    }

    return TRUE;
}
