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

.extern mapNudgeViewProc1ReturnAddress

.text
.globl mapNudgeViewProc1V137
mapNudgeViewProc1V137:
    cmp     byte ptr [rcx+rax], ESCAPE_SEQ_1
    jz      .LmapNudgeViewProc1_JMP_A
    cmp     byte ptr [rcx+rax], ESCAPE_SEQ_2
    jz      .LmapNudgeViewProc1_JMP_B
    cmp     byte ptr [rcx+rax], ESCAPE_SEQ_3
    jz      .LmapNudgeViewProc1_JMP_C
    cmp     byte ptr [rcx+rax], ESCAPE_SEQ_4
    jz      .LmapNudgeViewProc1_JMP_D

    movzx   eax, byte ptr [rcx+rax]
    jmp     .LmapNudgeViewProc1_JMP_F

.LmapNudgeViewProc1_JMP_A:
    movzx   eax, word ptr [rcx+rax + 1]
    jmp     .LmapNudgeViewProc1_JMP_E

.LmapNudgeViewProc1_JMP_B:
    movzx   eax, word ptr [rcx+rax + 1]
    sub     eax, SHIFT_2
    jmp     .LmapNudgeViewProc1_JMP_E

.LmapNudgeViewProc1_JMP_C:
    movzx   eax, word ptr [rcx+rax + 1]
    add     eax, SHIFT_3
    jmp     .LmapNudgeViewProc1_JMP_E

.LmapNudgeViewProc1_JMP_D:
    movzx   eax, word ptr [rcx+rax + 1]
    add     eax, SHIFT_4

.LmapNudgeViewProc1_JMP_E:
    movzx   eax, ax
    add     rcx, 2
    cmp     eax, NO_FONT
    ja      .LmapNudgeViewProc1_JMP_F
    mov     eax, NOT_DEF

.LmapNudgeViewProc1_JMP_F:
    mov     rdx, qword ptr [r12+rax*8+0x120]
    test    rdx, rdx

    mov     r11, qword ptr [rip + mapNudgeViewProc1ReturnAddress]
    push    r11
    ret
