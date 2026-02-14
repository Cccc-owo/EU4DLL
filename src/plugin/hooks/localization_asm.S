.intel_syntax noprefix

.set ESCAPE_SEQ_1, 0x10
.set ESCAPE_SEQ_2, 0x11
.set ESCAPE_SEQ_3, 0x12
.set ESCAPE_SEQ_4, 0x13
.set LOW_SHIFT, 0x0E
.set HIGH_SHIFT, 0x09
.set SHIFT_2, LOW_SHIFT
.set SHIFT_3, 0x900
.set SHIFT_4, 0x8F2
.set NO_FONT, 0x98F
.set NOT_DEF, 0x2026

.extern localizationProc2CallAddress
.extern localizationProc2ReturnAddress
.extern localizationProc5CallAddress
.extern localizationProc5ReturnAddress
.extern localizationProc6CallAddress
.extern localizationProc6ReturnAddress
.extern localizationProc8CallAddress
.extern localizationProc8ReturnAddress

.data
.globl dayA
dayA:
    .asciz "DD"
.globl monthA
monthA:
    .asciz "MONTH"
.globl yearA
yearA:
    .asciz "YYYY"

.text
.globl localizationProc2V137
localizationProc2V137:
    lea     rdx, [rbp-0x28]
    cmp     qword ptr [rbp-0x10], 0x10
    cmovnb  rdx, [rbp-0x28]
    mov     r8, [rbp-0x18]
    mov     rcx, rbx
    mov     r11, qword ptr [rip + localizationProc2CallAddress]
    call    r11

    mov     r11, qword ptr [rip + localizationProc2ReturnAddress]
    push    r11
    ret

.globl localizationProc5V137
localizationProc5V137:
    lea     rdx, [rbp-0x48]
    mov     rcx, rbx
    mov     r11, qword ptr [rip + localizationProc5CallAddress]
    call    r11

    mov     r11, qword ptr [rip + localizationProc5ReturnAddress]
    push    r11
    ret

.globl localizationProc6V137
localizationProc6V137:
    lea     r8, [rbp+0x57-0x78]
    lea     rdx, [rbp+0x57-0x98]
    lea     rcx, [rbp+0x57-0x38]
    mov     r11, qword ptr [rip + localizationProc6CallAddress]
    call    r11

    mov     r11, qword ptr [rip + localizationProc6ReturnAddress]
    push    r11
    ret

.globl localizationProc8V137
localizationProc8V137:
    nop
    lea     r8, [rsp+0xC8-0x88]
    mov     rdx, rax
    mov     rcx, rdi
    mov     r11, qword ptr [rip + localizationProc8CallAddress]
    call    r11

    mov     r11, qword ptr [rip + localizationProc8ReturnAddress]
    push    r11
    ret
