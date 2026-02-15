#include "byte_pattern.h"
#include "plugin_64.h"

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ulReasonForCall,
                      LPVOID lpReserved){

	if (ulReasonForCall == DLL_PROCESS_ATTACH){
		BytePattern::StartLog(L"eu4_jps_2");

		DllError e;

		RunOptions options = {};

		// INI settings
		Ini::GetOptionsFromIni(&options);

		// Version check (v1.37 only)
		if (Validator::ValidateVersion()) {

			// UTF-8 auto conversion (IAT hook)
			FileRead::Init(options);

			// Steam rich presence text fix (vtable hook)
			SteamRichPresence::Init();

			// Font loading
			e.merge(Font::Init(options));

			// UI text display
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

			Validator::Validate(e, options);
		}
	}else if (ulReasonForCall == DLL_PROCESS_DETACH){
		BytePattern::ShutdownLog();
	}

    return TRUE;
}
