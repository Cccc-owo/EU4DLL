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

.extern mapViewProc1ReturnAddress
.extern mapViewProc2ReturnAddress
.extern mapViewProc3ReturnAddress
.extern mapViewProc4ReturnAddress
.extern mapViewProc3CallAddress

.data
.globl mapViewProc3TmpCharacterAddress
mapViewProc3TmpCharacterAddress:
    .quad 0

.text
.globl mapViewProc1V137
mapViewProc1V137:
    movss   dword ptr [rbp + 0x1060 - 0x10C0], xmm3

    cmp     byte ptr [r9+rax], ESCAPE_SEQ_1
    jz      .LmapViewProc1_JMP_A
    cmp     byte ptr [r9+rax], ESCAPE_SEQ_2
    jz      .LmapViewProc1_JMP_B
    cmp     byte ptr [r9+rax], ESCAPE_SEQ_3
    jz      .LmapViewProc1_JMP_C
    cmp     byte ptr [r9+rax], ESCAPE_SEQ_4
    jz      .LmapViewProc1_JMP_D
    jmp     .LmapViewProc1_JMP_E

.LmapViewProc1_JMP_A:
    movzx   eax, word ptr [r9+rax + 1]
    jmp     .LmapViewProc1_JMP_F

.LmapViewProc1_JMP_B:
    movzx   eax, word ptr [r9+rax + 1]
    sub     eax, SHIFT_2
    jmp     .LmapViewProc1_JMP_F

.LmapViewProc1_JMP_C:
    movzx   eax, word ptr [r9+rax + 1]
    add     eax, SHIFT_3
    jmp     .LmapViewProc1_JMP_F

.LmapViewProc1_JMP_D:
    movzx   eax, word ptr [r9+rax + 1]
    add     eax, SHIFT_4

.LmapViewProc1_JMP_F:
    add     r11d, 2
    add     r9d, 2
    movzx   eax, ax
    cmp     eax, NO_FONT
    ja      .LmapViewProc1_JMP_G
    mov     eax, NOT_DEF
    jmp     .LmapViewProc1_JMP_G

.LmapViewProc1_JMP_E:
    movzx   eax, byte ptr [r9+rax]

.LmapViewProc1_JMP_G:
    mov     rdx, qword ptr [r15+rax*8]

    # issue-161
    cmp     rdx, 0
    jnz     .LmapViewProc1_JMP_N

    # issue-237
    cmp     rax, 0xFF
    jl      .LmapViewProc1_JMP_N

    mov     rax, 0x2D # '-'
    mov     rdx, qword ptr [r15+rax*8]

.LmapViewProc1_JMP_N:
    push    rax
    mov     rax, qword ptr [rip + mapViewProc1ReturnAddress]
    xchg    qword ptr [rsp], rax
    test    rdx, rdx
    ret

.globl mapViewProc2V137
mapViewProc2V137:
    cmp     byte ptr[r15+rax], ESCAPE_SEQ_1
    jz      .LmapViewProc2_JMP_A
    cmp     byte ptr[r15+rax], ESCAPE_SEQ_2
    jz      .LmapViewProc2_JMP_B
    cmp     byte ptr[r15+rax], ESCAPE_SEQ_3
    jz      .LmapViewProc2_JMP_C
    cmp     byte ptr[r15+rax], ESCAPE_SEQ_4
    jz      .LmapViewProc2_JMP_D

    movzx   eax, byte ptr [r15+rax]
    jmp     .LmapViewProc2_JMP_E

.LmapViewProc2_JMP_A:
    movzx   eax, word ptr[r15+rax + 1]
    jmp     .LmapViewProc2_JMP_F

.LmapViewProc2_JMP_B:
    movzx   eax, word ptr[r15+rax + 1]
    sub     eax, SHIFT_2
    jmp     .LmapViewProc2_JMP_F

.LmapViewProc2_JMP_C:
    movzx   eax, word ptr[r15+rax + 1]
    add     eax, SHIFT_3
    jmp     .LmapViewProc2_JMP_F

.LmapViewProc2_JMP_D:
    movzx   eax, word ptr[r15+rax + 1]
    add     eax, SHIFT_4

.LmapViewProc2_JMP_F:
    add     r14d, 2
    add     r15, 2

    movzx   eax, ax
    cmp     eax, NO_FONT
    ja      .LmapViewProc2_JMP_E
    mov     eax, NOT_DEF

.LmapViewProc2_JMP_E:
    mov     r11, qword ptr [r9+rax*8+0x120]
    push    rax
    mov     rax, qword ptr [rip + mapViewProc2ReturnAddress]
    xchg    qword ptr [rsp], rax
    test    r11, r11
    ret

.globl mapViewProc3V137
mapViewProc3V137:
    cmp     byte ptr [r15+rax], ESCAPE_SEQ_1
    jz      .LmapViewProc3_JMP_A
    cmp     byte ptr [r15+rax], ESCAPE_SEQ_2
    jz      .LmapViewProc3_JMP_A
    cmp     byte ptr [r15+rax], ESCAPE_SEQ_3
    jz      .LmapViewProc3_JMP_A
    cmp     byte ptr [r15+rax], ESCAPE_SEQ_4
    jz      .LmapViewProc3_JMP_A

    movzx   r8d, byte ptr[r15+rax]
    mov     edx, 1
    lea     rcx, qword ptr [rsp+0x1160-0x1108]
    mov     r11, qword ptr [rip + mapViewProc3CallAddress]
    call    r11

    jmp     .LmapViewProc3_JMP_B
.LmapViewProc3_JMP_A:
    lea     r8, qword ptr [r15+rax]
    mov     qword ptr [rip + mapViewProc3TmpCharacterAddress], r8
    movzx   r8d, byte ptr[r15+rax]
    mov     edx, 3
    lea     rcx, qword ptr [rsp+0x1160-0x1108]
    mov     r11, qword ptr [rip + mapViewProc3CallAddress]
    call    r11

    # overwrite
    mov     rcx, qword ptr [rip + mapViewProc3TmpCharacterAddress]
    mov     cx, word ptr [rcx+1]
    mov     word ptr [rax+1], cx

.LmapViewProc3_JMP_B:
    push    rax
    mov     rax, qword ptr [rip + mapViewProc3ReturnAddress]
    xchg    qword ptr [rsp], rax
    ret

.globl mapViewProc4V137
mapViewProc4V137:
    lea     rcx, qword ptr [rsp+0x1160-0x10E8]
    cmp     rbx, 0x10
    cmovnb  rcx, rsi
    movzx   r8d, byte ptr [rax+rcx]
    lea     rax, qword ptr [rsp+0x1160-0x10E8]
    cmovnb  rax, rsi
    movzx   edx, byte ptr [rax+r9]
    mov     rcx, r15

    push    rax
    mov     rax, qword ptr [rip + mapViewProc4ReturnAddress]
    xchg    qword ptr [rsp], rax
    ret
