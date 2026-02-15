#include "byte_pattern.h"
#include "plugin_64.h"

namespace Validator {

	bool Validate(DllError e, RunOptions options) {
		auto message = e.print();

		BytePattern::LoggingInfo(message);

		if (e.hasError()) {
			const WCHAR* caption = L"ERROR";
			const WCHAR* msg = L"Multibyte DLL hasn't supported this game version yet.\n"
				  L"\n"
				  L"DLL announce page:\n"
				  L"https://github.com/Cccc-owo/EU4dll";

			// Try MessageBox, but don't fail hard if it doesn't work (Wine without X11)
			MessageBoxW(NULL, msg, caption, MB_OK);

			BytePattern::LoggingInfo("DLL [NG]");

			return false;
		}
		else {
			BytePattern::LoggingInfo("DLL [OK]");
			return true;
		}
	}

	bool ValidateVersion() {
		bool detected = Version::DetectVersion();

		if (!detected) {
			const WCHAR* caption = L"UNSUPPORTED VERSION";
			const WCHAR* msg = L"Multibyte DLL hasn't supported this game version yet.\n"
				  L"Do you want to start the game anyway?";

			int result = MessageBoxW(NULL, msg, caption, MB_YESNO);

			if (result == IDYES) {
				BytePattern::LoggingInfo("DLL [SKIP]");
				return false;
			}
			else {
				BytePattern::LoggingInfo("DLL [VERSION MISMATCH]");
				return false;
			}
		}
		else {
			BytePattern::LoggingInfo("DLL [MATCH VERSION]");
			return true;
		}
	}
}
