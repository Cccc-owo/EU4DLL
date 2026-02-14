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

.extern listFieldAdjustmentProc1ReturnAddress
.extern listFieldAdjustmentProc2ReturnAddress
.extern listFieldAdjustmentProc3ReturnAddressA
.extern listFieldAdjustmentProc3ReturnAddressB

.data
.globl listFieldAdjustmentProc1MultibyteFlag
listFieldAdjustmentProc1MultibyteFlag:
    .long 0
.globl listFieldAdjustmentProc2length
listFieldAdjustmentProc2length:
    .long 0

.text
.globl listFieldAdjustmentProc1V137
listFieldAdjustmentProc1V137:
    mov     r8, [rbp+0x100-0x148]
    movss   xmm6, dword ptr [r8+0x848]

    cmp     byte ptr[rax+rdx], ESCAPE_SEQ_1
    jz      .LlfaProc1_JMP_A
    cmp     byte ptr[rax+rdx], ESCAPE_SEQ_2
    jz      .LlfaProc1_JMP_B
    cmp     byte ptr[rax+rdx], ESCAPE_SEQ_3
    jz      .LlfaProc1_JMP_C
    cmp     byte ptr[rax+rdx], ESCAPE_SEQ_4
    jz      .LlfaProc1_JMP_D

    movzx   eax, byte ptr [rax+rdx]
    jmp     .LlfaProc1_JMP_G

.LlfaProc1_JMP_A:
    movzx   eax, word ptr[rax+rdx + 1]
    jmp     .LlfaProc1_JMP_E

.LlfaProc1_JMP_B:
    movzx   eax, word ptr[rax+rdx + 1]
    sub     eax, SHIFT_2
    jmp     .LlfaProc1_JMP_E

.LlfaProc1_JMP_C:
    movzx   eax, word ptr[rax+rdx + 1]
    add     eax, SHIFT_3
    jmp     .LlfaProc1_JMP_E

.LlfaProc1_JMP_D:
    movzx   eax, word ptr[rax+rdx + 1]
    add     eax, SHIFT_4
    jmp     .LlfaProc1_JMP_E

.LlfaProc1_JMP_E:
    mov     dword ptr [rip + listFieldAdjustmentProc1MultibyteFlag], 0x1
    movzx   eax, ax
    cmp     eax, NO_FONT
    ja      .LlfaProc1_JMP_G
    mov     eax, NOT_DEF

.LlfaProc1_JMP_G:
    mov     r12, qword ptr [r8+rax*8]
    test    r12, r12

    mov     r11, qword ptr [rip + listFieldAdjustmentProc1ReturnAddress]
    push    r11
    ret

.globl listFieldAdjustmentProc2V137
listFieldAdjustmentProc2V137:
    cmp     dword ptr [rip + listFieldAdjustmentProc1MultibyteFlag], 0x1
    jnz     .LlfaProc2_JMP_A
    add     ebx, 2

.LlfaProc2_JMP_A:
    inc     ebx
    mov     dword ptr [rip + listFieldAdjustmentProc1MultibyteFlag], 0x0

    mov     esi, ebx
    mov     ecx, dword ptr [rdi+0x10]
    movzx   r9d, byte ptr [rbp+0x100 + 0x48]
    xor     r10d, r10d
    cmp     ebx, ecx

    mov     dword ptr [rip + listFieldAdjustmentProc2length], ebx

    mov     r11, qword ptr [rip + listFieldAdjustmentProc2ReturnAddress]
    push    r11
    ret

.globl listFieldAdjustmentProc3V137A
listFieldAdjustmentProc3V137A:
    mov     rcx, qword ptr [rax+rcx*8]
    mov     r9d, dword ptr [rcx+rdx*4]

    cmp     dword ptr [rip + listFieldAdjustmentProc1MultibyteFlag], 0x1
    jnz     .LlfaProc3A_JMP_A
    mov     r9d, dword ptr [rip + listFieldAdjustmentProc2length]
    sub     r9d, 4

.LlfaProc3A_JMP_A:
    xor     r8d, r8d
    lea     rdx, qword ptr [rbp+0x100-0x168]
    mov     rcx, rdi

    mov     r11, qword ptr [rip + listFieldAdjustmentProc3ReturnAddressA]
    push    r11
    ret

.globl listFieldAdjustmentProc3V137B
listFieldAdjustmentProc3V137B:
    mov     rcx, qword ptr [rax+rcx*8]
    mov     r9d, dword ptr [rcx+rdx*4]

    cmp     dword ptr [rip + listFieldAdjustmentProc1MultibyteFlag], 0x1
    jnz     .LlfaProc3B_JMP_A
    mov     r9d, dword ptr [rip + listFieldAdjustmentProc2length]
    sub     r9d, 4

.LlfaProc3B_JMP_A:
    xor     r8d, r8d
    lea     rdx, qword ptr [rbp+0x100-0x168]
    mov     rcx, rdi

    mov     r11, qword ptr [rip + listFieldAdjustmentProc3ReturnAddressB]
    push    r11
    ret
