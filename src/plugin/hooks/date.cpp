#include "../plugin_64.h"

namespace Date {

	DllError Init(RunOptions options) {
		DllError e = {};

		// "d w m" pattern - DateFormat
		BytePattern::temp_instance().find_pattern("64 20 77 20 6D");
		if (BytePattern::temp_instance().has_size(1, "DateFormat")) {
			uintptr_t address = BytePattern::temp_instance().get_first().address();

			// y 0x0F m w d 0x0E  →  ex) 1444年11月11日
			char dateFormat[] = { 'y', ' ', 0x0F, ' ', 'm', 'w', ' ', 'd', ' ', 0x0E, 0 };
			Injector::WriteMemoryRaw(address, dateFormat, sizeof(dateFormat), true);
		}
		else {
			e.date.unmatchdDateProc1Injector = true;
		}

		return e;
	}
}
