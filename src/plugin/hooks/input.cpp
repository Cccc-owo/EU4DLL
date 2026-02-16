#include "../byte_pattern.h"
#include "../escape_tool.h"
#include "../injector.h"
#include "../plugin_64.h"

namespace Input {
    extern "C" {
    void      inputProc1V137();
    void      inputProc2V137();
    uintptr_t inputProc1ReturnAddress1;
    uintptr_t inputProc1ReturnAddress2;
    uintptr_t inputProc1CallAddress;
    uintptr_t inputProc2ReturnAddress;
    }

    bool inputProc1Injector() {
        bool failed = false;

        // mov     eax, dword ptr [rbp+120h+var_18C]
        BytePattern::temp_instance().find_pattern("8B 45 BC 32 DB 3C 80 73 05 0F B6 D8 EB 12");
        if (BytePattern::temp_instance().has_size(
                1, "convert input char from UTF-8 to escape sequence 1")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();

            inputProc1CallAddress = (uintptr_t)utf8ToEscapedStr3;

            // mov     rax, [r15]
            inputProc1ReturnAddress1 = address + 0x20;

            Injector::MakeJMP(address, inputProc1V137, true);
        } else {
            failed = true;
        }

        // call    qword ptr [rax+18h]
        BytePattern::temp_instance().find_pattern("FF 50 18 E9 ? ? ? ? 49 8B 07 45 33 C9");
        if (BytePattern::temp_instance().has_size(
                1, "convert input char from UTF-8 to escape sequence 2")) {
            uintptr_t address = BytePattern::temp_instance().get_first().address();
            // jmp     loc_{xxxxx}
            inputProc1ReturnAddress2 = Injector::GetBranchDestination(address + 0x3);
        } else {
            failed = true;
        }

        return failed;
    }

    bool inputProc2Injector() {
        // xor     ecx, ecx
        BytePattern::temp_instance().find_pattern(
            "33 C9 48 89 4C 24 20 48 C7 44 24 38 0F 00 00 00 48 89 4C 24 30");
        if (BytePattern::temp_instance().has_size(3, "backspace handling fix")) {
            uintptr_t address = BytePattern::temp_instance().get(2).address();

            // movzx   r8d, word ptr [rdi+56h]
            inputProc2ReturnAddress = address + 0x165;

            Injector::MakeJMP(address, inputProc2V137, true);
            return false;
        }
        return true;
    }

    HookResult Init(const RunOptions& options) {
        HookResult result("Input");

        result.add("inputProc1Injector", inputProc1Injector());
        result.add("inputProc2Injector", inputProc2Injector());

        return result;
    }
} // namespace Input
