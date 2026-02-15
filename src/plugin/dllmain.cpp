#include "plugin64.h"
#include "plugin_64.h"

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ulReasonForCall,
                      LPVOID lpReserved){

	if (ulReasonForCall == DLL_PROCESS_ATTACH){
		BytePattern::StartLog(L"eu4_jps_2");

		DllError e = {};

		RunOptions options = {};

		// INI settings
		Ini::GetOptionsFromIni(&options);

		// Version check (v1.37 only)
		if (Validator::ValidateVersion()) {

			// UTF-8 auto conversion (IAT hook)
			FileRead::Init(options);

			// Font loading
			e |= Font::Init(options);

			// UI text display
			e |= MainText::Init(options);

			// Tooltip and button display
			e |= TooltipAndButton::Init(options);

			// Map text display
			e |= MapView::Init(options);

			// Map text display (nudge)
			e |= MapNudgeView::Init(options);

			// Map text adjustment
			e |= MapAdjustment::Init(options);

			// Map text justify
			e |= MapJustify::Init(options);

			// Event dialog and map text adjustment
			e |= EventDialog::Init(options);

			// Map popup text display
			e |= MapPopup::Init(options);

			// List field adjustment
			e |= ListFieldAdjustment::Init(options);

			// File save
			e |= FileSave::Init(options);

			// Date format
			e |= Date::Init(options);

			// IME
			e |= Ime::Init(options);

			// Input
			e |= Input::Init(options);

			// Localization string reorder
			e |= Localization::Init(options);

			// BitmapFont adjustment, scroll adjustment
			e |= CBitmapFont::Init(options);

			Validator::Validate(e, options);
		}
	}else if (ulReasonForCall == DLL_PROCESS_DETACH){
		BytePattern::ShutdownLog();
	}

    return TRUE;
}
