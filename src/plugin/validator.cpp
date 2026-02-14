#include "plugin64.h"
#include "plugin_64.h"

namespace Validator {

	void Validate(DllError e, RunOptions options) {
		auto message = e.print();

		BytePattern::LoggingInfo(message);

		if (e.errorCheck()) {
			const WCHAR* msg;
			const WCHAR* caption;

			const DWORD sysDefLcid = ::GetSystemDefaultLCID();

			switch (sysDefLcid) {
			case MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN):
				caption = L"\x30A8\x30E9\x30FC"; // エラー
				msg = L"\x3053\x306E\x30D0\x30FC\x30B8\x30E7\x30F3\x306F"
					  L"\x307E\x3060\x65E5\x672C\x8A9E\x5316\x306B\x5BFE\x5FDC"
					  L"\x3057\x3066\x3044\x306A\x3044\x305F\x3081\x8D77\x52D5"
					  L"\x3067\x304D\x307E\x305B\x3093\x3002\n"
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

			exit(-1);
		}
		else {
			BytePattern::LoggingInfo("DLL [OK]");
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
				caption = L"\x672A\x5BFE\x5FDC\x30D0\x30FC\x30B8\x30E7\x30F3"; // 未対応バージョン
				msg = L"\x65E5\x672C\x8A9E\x5316" L"dll"
					  L"\x306F\x3053\x306E\x30D0\x30FC\x30B8\x30E7\x30F3\x306B"
					  L"\x672A\x5BFE\x5FDC\x3067\x3059\x3002"
					  L"\x8D77\x52D5\x3092\x512A\x5148\x3057\x307E\x3059\x304B\xFF1F"; // 起動を優先しますか？
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
				exit(-1);
			}
		}
		else {
			BytePattern::LoggingInfo("DLL [MATCH VERSION]");
			return true;
		}
	}
}
