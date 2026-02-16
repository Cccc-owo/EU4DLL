#include "../byte_pattern.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace Date {

    HookResult Init(const RunOptions& options) {
        HookResult result("Date");

        // "d w m" pattern - DateFormat
        BytePattern::temp_instance().find_pattern("64 20 77 20 6D");
        bool failed = true;
        if (BytePattern::temp_instance().has_size(1, "DateFormat")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            // y 0x0F m w d 0x0E  →  ex) 1444y11m11d
            char dateFormat[] = {'y', ' ', 0x0F, ' ', 'm', 'w', ' ', 'd', ' ', 0x0E, 0};
            Injector::WriteMemoryRaw(address, dateFormat, sizeof(dateFormat), true);
            failed = false;
        }
        result.add("DateProc1Injector", failed);

        return result;
    }
} // namespace Date
