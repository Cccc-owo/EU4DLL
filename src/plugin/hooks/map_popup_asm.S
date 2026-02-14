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

.extern mapPopupProc1ReturnAddress
.extern mapPopupProc1CallAddressA
.extern mapPopupProc1CallAddressB
.extern mapPopupProc2ReturnAddress
.extern mapPopupProc3ReturnAddress

.data
.globl mapPopupProc1TmpCharacterAddress
mapPopupProc1TmpCharacterAddress:
    .quad 0

.text
.globl mapPopupProc1V137
mapPopupProc1V137:
    lea     rcx, [rbp+0x370-0x320]

    cmp     bl, ESCAPE_SEQ_1
    jz      .LmapPopupProc1_JMP_A
    cmp     bl, ESCAPE_SEQ_2
    jz      .LmapPopupProc1_JMP_A
    cmp     bl, ESCAPE_SEQ_3
    jz      .LmapPopupProc1_JMP_A
    cmp     bl, ESCAPE_SEQ_4
    jz      .LmapPopupProc1_JMP_A

    mov     r11, qword ptr [rip + mapPopupProc1CallAddressA]
    call    r11
    nop
    movzx   r8d, bl
    mov     edx, 1
    lea     rcx, qword ptr [rbp+0x370-0x320]
    mov     r11, qword ptr [rip + mapPopupProc1CallAddressB]
    call    r11
    jmp     .LmapPopupProc1_JMP_B

.LmapPopupProc1_JMP_A:
    lea     rdx, qword ptr [rdi + rax]
    mov     qword ptr [rip + mapPopupProc1TmpCharacterAddress], rdx
    mov     r11, qword ptr [rip + mapPopupProc1CallAddressA]
    call    r11
    nop
    movzx   r8d, bl
    mov     edx, 3
    lea     rcx, qword ptr [rbp+0x370-0x320]
    mov     r11, qword ptr [rip + mapPopupProc1CallAddressB]
    call    r11

    # overwrite
    mov     rcx, qword ptr [rip + mapPopupProc1TmpCharacterAddress]
    mov     ecx, dword ptr [rcx]
    mov     dword ptr [rax], ecx

.LmapPopupProc1_JMP_B:
    mov     r11, qword ptr [rip + mapPopupProc1ReturnAddress]
    push    r11
    ret

.globl mapPopupProc2V137
mapPopupProc2V137:
    cmp     byte ptr[rdi+rax], ESCAPE_SEQ_1
    jz      .LmapPopupProc2_JMP_A
    cmp     byte ptr[rdi+rax], ESCAPE_SEQ_2
    jz      .LmapPopupProc2_JMP_B
    cmp     byte ptr[rdi+rax], ESCAPE_SEQ_3
    jz      .LmapPopupProc2_JMP_C
    cmp     byte ptr[rdi+rax], ESCAPE_SEQ_4
    jz      .LmapPopupProc2_JMP_D

    movzx   eax, byte ptr [rdi+rax]
    jmp     .LmapPopupProc2_JMP_H

.LmapPopupProc2_JMP_A:
    movzx   eax, word ptr[rdi+rax + 1]
    jmp     .LmapPopupProc2_JMP_E

.LmapPopupProc2_JMP_B:
    movzx   eax, word ptr[rdi+rax + 1]
    sub     eax, SHIFT_2
    jmp     .LmapPopupProc2_JMP_E

.LmapPopupProc2_JMP_C:
    movzx   eax, word ptr[rdi+rax + 1]
    add     eax, SHIFT_3
    jmp     .LmapPopupProc2_JMP_E

.LmapPopupProc2_JMP_D:
    movzx   eax, word ptr[rdi+rax + 1]
    add     eax, SHIFT_4

.LmapPopupProc2_JMP_E:
    movzx   eax, ax
    cmp     eax, NO_FONT
    ja      .LmapPopupProc2_JMP_G
    mov     eax, NOT_DEF

.LmapPopupProc2_JMP_G:
    add     edi, 2

.LmapPopupProc2_JMP_H:
    mov     r12, qword ptr [r15+rax*8+0x120]
    test    r12, r12

    mov     r11, qword ptr [rip + mapPopupProc2ReturnAddress]
    push    r11
    ret

.globl mapPopupProc3V137
mapPopupProc3V137:
    cmp     byte ptr[rsi+rax], ESCAPE_SEQ_1
    jz      .LmapPopupProc3_JMP_A
    cmp     byte ptr[rsi+rax], ESCAPE_SEQ_2
    jz      .LmapPopupProc3_JMP_B
    cmp     byte ptr[rsi+rax], ESCAPE_SEQ_3
    jz      .LmapPopupProc3_JMP_C
    cmp     byte ptr[rsi+rax], ESCAPE_SEQ_4
    jz      .LmapPopupProc3_JMP_D

    movzx   eax, byte ptr [rsi+rax]
    jmp     .LmapPopupProc3_JMP_H

.LmapPopupProc3_JMP_A:
    movzx   eax, word ptr[rsi+rax + 1]
    jmp     .LmapPopupProc3_JMP_E

.LmapPopupProc3_JMP_B:
    movzx   eax, word ptr[rsi+rax + 1]
    sub     eax, SHIFT_2
    jmp     .LmapPopupProc3_JMP_E

.LmapPopupProc3_JMP_C:
    movzx   eax, word ptr[rsi+rax + 1]
    add     eax, SHIFT_3
    jmp     .LmapPopupProc3_JMP_E

.LmapPopupProc3_JMP_D:
    movzx   eax, word ptr[rsi+rax + 1]
    add     eax, SHIFT_4

.LmapPopupProc3_JMP_E:
    movzx   eax, ax
    cmp     eax, NO_FONT
    ja      .LmapPopupProc3_JMP_G
    mov     eax, NOT_DEF

.LmapPopupProc3_JMP_G:
    add     esi, 2

.LmapPopupProc3_JMP_H:
    mov     r13, qword ptr [r15+rax*8+0x120]
    test    r13, r13

    mov     r11, qword ptr [rip + mapPopupProc3ReturnAddress]
    push    r11
    ret
