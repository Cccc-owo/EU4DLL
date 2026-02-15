#include "byte_pattern.h"
#include "injector.h"
#include "plugin_64.h"

namespace Version {
	typedef struct _Pattern {
		char ascii1;
		char ascii2;
		char dot;
		char ascii3;

		int calVer() {
			int ver = (ascii1 - 0x30) * 100 + (ascii2 - 0x30) * 10 + (ascii3 - 0x30);
			return ver;
		}
	} Pattern;

	bool DetectVersion() {
		// EU4 v1.??.?
		BytePattern::temp_instance().find_pattern("45 55 34 20 76 31 2E ? ? 2E ?");
		if (BytePattern::temp_instance().count() > 0) {
			Pattern minor = Injector::ReadMemory<Pattern>(BytePattern::temp_instance().get_first().address(0x7), true);

			int ver = minor.calVer();
			// Accept v1.37.x (370-375)
			if (ver >= 370 && ver <= 375) {
				BytePattern::LoggingInfo("v1_37_0_0");
				return true;
			}
		}

		BytePattern::LoggingInfo("UNKNOWN version");
		return false;
	}
}
