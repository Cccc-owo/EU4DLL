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
.set MAP_LIMIT, 0x2D-3

.extern mapJustifyProc1ReturnAddress1
.extern mapJustifyProc1ReturnAddress2
.extern mapJustifyProc2ReturnAddress
.extern mapJustifyProc4ReturnAddress

.data
.globl mapJustifyProc1TmpFlag
mapJustifyProc1TmpFlag:
    .long 0
.globl mapJustifyDebug
mapJustifyDebug:
    .quad 0

.text
.globl mapJustifyProc1V137
mapJustifyProc1V137:
    movss   xmm12, dword ptr [rdx+0x848]

    cmp     byte ptr [rcx+rax], ESCAPE_SEQ_1
    jz      .LmapJustifyProc1_JMP_A
    cmp     byte ptr [rcx+rax], ESCAPE_SEQ_2
    jz      .LmapJustifyProc1_JMP_B
    cmp     byte ptr [rcx+rax], ESCAPE_SEQ_3
    jz      .LmapJustifyProc1_JMP_C
    cmp     byte ptr [rcx+rax], ESCAPE_SEQ_4
    jz      .LmapJustifyProc1_JMP_D
    movzx   eax, byte ptr [rcx+rax]
    mov     byte ptr [rbp+0x7B0+0x58], al
    mov     dword ptr [rip + mapJustifyProc1TmpFlag], 0x0
    jmp     .LmapJustifyProc1_JMP_I

.LmapJustifyProc1_JMP_A:
    movzx   eax, word ptr [rcx+rax + 1]
    jmp     .LmapJustifyProc1_JMP_F

.LmapJustifyProc1_JMP_B:
    movzx   eax, word ptr [rcx+rax + 1]
    sub     eax, SHIFT_2
    jmp     .LmapJustifyProc1_JMP_F

.LmapJustifyProc1_JMP_C:
    movzx   eax, word ptr [rcx+rax + 1]
    add     eax, SHIFT_3
    jmp     .LmapJustifyProc1_JMP_F

.LmapJustifyProc1_JMP_D:
    movzx   eax, word ptr [rcx+rax + 1]
    add     eax, SHIFT_4

.LmapJustifyProc1_JMP_F:
    cmp     rcx, MAP_LIMIT
    ja      .LmapJustifyProc1_JMP_H

    movzx   eax, ax
    cmp     eax, NO_FONT
    ja      .LmapJustifyProc1_JMP_G
.LmapJustifyProc1_JMP_H:
    mov     eax, NOT_DEF

.LmapJustifyProc1_JMP_G:
    mov     dword ptr [rip + mapJustifyProc1TmpFlag], 0x1
    mov     byte ptr [rbp+0x7B0+0x58], ESCAPE_SEQ_1

.LmapJustifyProc1_JMP_I:
    mov     r14, qword ptr [rdx+rax*8]
    test    r14, r14
    mov     r11, qword ptr [rip + mapJustifyProc1ReturnAddress1]
    push    r11
    ret

.globl mapJustifyProc2V137
mapJustifyProc2V137:
    mov     rax, [rbp+0x7B0-0x648]

    cmp     dword ptr [rip + mapJustifyProc1TmpFlag], 0x1
    jz      .LmapJustifyProc2_JMP_B

    dec     eax
    jmp     .LmapJustifyProc2_JMP_C

.LmapJustifyProc2_JMP_B:
    cmp     rax, 3
    ja      .LmapJustifyProc2_JMP_A
    inc     rax
    mov     esi, 1

.LmapJustifyProc2_JMP_A:
    dec     eax
    dec     eax

.LmapJustifyProc2_JMP_C:
    movd    xmm6, esi
    cvtdq2ps xmm6, xmm6
    movd    xmm1, eax

    mov     r11, qword ptr [rip + mapJustifyProc2ReturnAddress]
    push    r11
    ret

.globl mapJustifyProc4V137
mapJustifyProc4V137:
    cmp     dword ptr [rip + mapJustifyProc1TmpFlag], 0x1
    jnz     .LmapJustifyProc4_JMP_A

    add     esi, 3
    add     rcx, 3

    jmp     .LmapJustifyProc4_JMP_C

.LmapJustifyProc4_JMP_A:
    inc     esi
    inc     rcx

.LmapJustifyProc4_JMP_C:
    mov     dword ptr [rbp+0x7B0+0x38], esi
    mov     qword ptr [rbp+0x7B0-0x698], rcx

    mov     r11, qword ptr [rip + mapJustifyProc4ReturnAddress]
    push    r11
    ret
