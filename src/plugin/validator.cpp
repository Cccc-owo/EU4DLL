#include "plugin64.h"
#include "plugin_64.h"

namespace Validator {

	bool Validate(DllError e, RunOptions options) {
		auto message = e.print();

		BytePattern::LoggingInfo(message);

		if (e.hasError()) {
			const WCHAR* msg;
			const WCHAR* caption;

			const DWORD sysDefLcid = ::GetSystemDefaultLCID();

			switch (sysDefLcid) {
			case MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN):
				caption = L"エラー";
				msg = L"このバージョンはまだ日本語化に対応していないため起動できません。\n"
					  L"https://github.com/matanki-saito/EU4dll";
				break;

			case MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US):
			default:
				caption = L"ERROR";
				msg = L"Multibyte DLL hasn't supported this game version yet.\n"
					  L"\n"
					  L"DLL announce page:\n"
					  L"https://github.com/matanki-saito/EU4dll";
			}

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
			const WCHAR* msg;
			const WCHAR* caption;

			const DWORD sysDefLcid = ::GetSystemDefaultLCID();

			switch (sysDefLcid) {
			case MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN):
				caption = L"未対応バージョン";
				msg = L"日本語化dllはこのバージョンに未対応です。"
					  L"起動を優先しますか？";
				break;

			case MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US):
			default:
				caption = L"NO SUPPORT VERSION";
				msg = L"Multibyte DLL hasn't supported this game version yet.\n"
					  L"Do you want to start a game?";
				break;
			}

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
