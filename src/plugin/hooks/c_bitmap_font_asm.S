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

.extern cBitmapFontProc1ReturnAddress
.extern cBitmapFontProc2ReturnAddress

.text
.globl cBitmapFontProc1V137
cBitmapFontProc1V137:
    cmp     byte ptr[rdi + rax], ESCAPE_SEQ_1
    jz      .LcbfProc1_JMP_A
    cmp     byte ptr[rdi + rax], ESCAPE_SEQ_2
    jz      .LcbfProc1_JMP_B
    cmp     byte ptr[rdi + rax], ESCAPE_SEQ_3
    jz      .LcbfProc1_JMP_C
    cmp     byte ptr[rdi + rax], ESCAPE_SEQ_4
    jz      .LcbfProc1_JMP_D

    movzx   eax, byte ptr [rdi + rax]
    jmp     .LcbfProc1_JMP_H

.LcbfProc1_JMP_A:
    movzx   eax, word ptr[rdi + rax + 1]
    jmp     .LcbfProc1_JMP_E

.LcbfProc1_JMP_B:
    movzx   eax, word ptr[rdi + rax + 1]
    sub     eax, SHIFT_2
    jmp     .LcbfProc1_JMP_E

.LcbfProc1_JMP_C:
    movzx   eax, word ptr[rdi + rax + 1]
    add     eax, SHIFT_3
    jmp     .LcbfProc1_JMP_E

.LcbfProc1_JMP_D:
    movzx   eax, word ptr[rdi + rax + 1]
    add     eax, SHIFT_4

.LcbfProc1_JMP_E:
    movzx   eax, ax
    cmp     eax, NO_FONT
    ja      .LcbfProc1_JMP_G
    mov     eax, NOT_DEF

.LcbfProc1_JMP_G:
    add     edi, 2
    xorps   xmm6, xmm6

.LcbfProc1_JMP_H:
    mov     rcx, qword ptr [r14 + rax * 8 + 0x120]
    test    rcx, rcx

    mov     r11, qword ptr [rip + cBitmapFontProc1ReturnAddress]
    push    r11
    ret

.globl cBitmapFontProc2V137
cBitmapFontProc2V137:
    movss   xmm6, dword ptr [r14+0x848]

    cmp     byte ptr[rdx+rax], ESCAPE_SEQ_1
    jz      .LcbfProc2_JMP_A
    cmp     byte ptr[rdx+rax], ESCAPE_SEQ_2
    jz      .LcbfProc2_JMP_B
    cmp     byte ptr[rdx+rax], ESCAPE_SEQ_3
    jz      .LcbfProc2_JMP_C
    cmp     byte ptr[rdx+rax], ESCAPE_SEQ_4
    jz      .LcbfProc2_JMP_D

    movzx   eax, byte ptr [rdx+rax]
    jmp     .LcbfProc2_JMP_H

.LcbfProc2_JMP_A:
    movzx   eax, word ptr[rdx+rax + 1]
    jmp     .LcbfProc2_JMP_E

.LcbfProc2_JMP_B:
    movzx   eax, word ptr[rdx+rax + 1]
    sub     eax, SHIFT_2
    jmp     .LcbfProc2_JMP_E

.LcbfProc2_JMP_C:
    movzx   eax, word ptr[rdx+rax + 1]
    add     eax, SHIFT_3
    jmp     .LcbfProc2_JMP_E

.LcbfProc2_JMP_D:
    movzx   eax, word ptr[rdx+rax + 1]
    add     eax, SHIFT_4

.LcbfProc2_JMP_E:
    movzx   eax, ax
    cmp     eax, NO_FONT
    ja      .LcbfProc2_JMP_G
    mov     eax, NOT_DEF

.LcbfProc2_JMP_G:
    add     edi, 2

.LcbfProc2_JMP_H:
    mov     r15, qword ptr [r14+rax*8]
    test    r15, r15

    mov     r11, qword ptr [rip + cBitmapFontProc2ReturnAddress]
    push    r11
    ret
