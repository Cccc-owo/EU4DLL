#include "byte_pattern.h"
#include "plugin_64.h"

namespace Validator {

	bool Validate(const DllError& e, const RunOptions& options) {
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
				  L"The game will start without the multibyte patch.";

			MessageBoxW(NULL, msg, caption, MB_OK);

			BytePattern::LoggingInfo("DLL [VERSION MISMATCH]");
			return false;
		}

		BytePattern::LoggingInfo("DLL [MATCH VERSION]");
		return true;
	}
}
