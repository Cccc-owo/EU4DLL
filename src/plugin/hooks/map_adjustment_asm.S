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

.extern mapAdjustmentProc1ReturnAddressA
.extern mapAdjustmentProc1ReturnAddressB
.extern mapAdjustmentProc1CallAddressA
.extern mapAdjustmentProc1CallAddressB
.extern mapAdjustmentProc2ReturnAddress
.extern mapAdjustmentProc3ReturnAddress1
.extern mapAdjustmentProc3ReturnAddress2
.extern mapAdjustmentProc4ReturnAddress

.data
.globl DefaultSeparator
DefaultSeparator:
    .ascii " \0"

.text
.globl mapAdjustmentProc1V137A
mapAdjustmentProc1V137A:
    movsx   ecx, byte ptr [rax+rbp]
    cmp		ecx, ESCAPE_SEQ_1
    jz		.LmapAdjProc1A_JMP_A
    cmp		ecx, ESCAPE_SEQ_2
    jz		.LmapAdjProc1A_JMP_A
    cmp		ecx, ESCAPE_SEQ_3
    jz		.LmapAdjProc1A_JMP_A
    cmp		ecx, ESCAPE_SEQ_4
    jz		.LmapAdjProc1A_JMP_A

    lea     rbx, [rax+rbp]
    mov		r11, qword ptr [rip + mapAdjustmentProc1CallAddressA]
    call	r11
    inc     edi
    mov     [rbx], al

    jmp		.LmapAdjProc1A_JMP_B

.LmapAdjProc1A_JMP_A:
    add		edi, 3
.LmapAdjProc1A_JMP_B:
    mov     eax, edi

    mov		r11, qword ptr [rip + mapAdjustmentProc1ReturnAddressA]
    push	r11
    ret

.globl mapAdjustmentProc1V137B
mapAdjustmentProc1V137B:
    movsx   ecx, byte ptr [rax+rbp]
    cmp		ecx, ESCAPE_SEQ_1
    jz		.LmapAdjProc1B_JMP_A
    cmp		ecx, ESCAPE_SEQ_2
    jz		.LmapAdjProc1B_JMP_A
    cmp		ecx, ESCAPE_SEQ_3
    jz		.LmapAdjProc1B_JMP_A
    cmp		ecx, ESCAPE_SEQ_4
    jz		.LmapAdjProc1B_JMP_A

    lea     rbx, [rax+rbp]
    mov		r11, qword ptr [rip + mapAdjustmentProc1CallAddressB]
    call	r11
    inc     edi
    mov     [rbx], al

    jmp		.LmapAdjProc1B_JMP_B

.LmapAdjProc1B_JMP_A:
    add		edi, 3
.LmapAdjProc1B_JMP_B:
    mov     eax, edi

    mov		r11, qword ptr [rip + mapAdjustmentProc1ReturnAddressB]
    push	r11
    ret

.globl mapAdjustmentProc2V137
mapAdjustmentProc2V137:
    lea     rax, qword ptr [rbp+0x230-0x1A0] # 1A0 = Block
    cmp     r13, 0x10
    cmovnb  rax, rsi
    movzx   eax, byte ptr [rax+rbx]
    mov     byte ptr [rbp+0x230-0x230], al

    cmp		al, ESCAPE_SEQ_1
    jz		.LmapAdjProc2_JMP_A
    cmp		al, ESCAPE_SEQ_2
    jz		.LmapAdjProc2_JMP_A
    cmp		al, ESCAPE_SEQ_3
    jz		.LmapAdjProc2_JMP_A
    cmp		al, ESCAPE_SEQ_4
    jz		.LmapAdjProc2_JMP_A

    lea     rax, qword ptr [rbp + 0x230 - 0x230]
    mov     r8, 0xFFFFFFFFFFFFFFFF

.LmapAdjProc2_JMP_B:
    inc     r8
    cmp     byte ptr [rax+r8], 0
    jnz		.LmapAdjProc2_JMP_B
    jmp		.LmapAdjProc2_JMP_C

.LmapAdjProc2_JMP_A:
    mov		r8, 3

    lea     rax, qword ptr [rbp+0x230-0x1A0] # 1A0 = Block
    cmp     r13, 0x10
    cmovnb  rax, rsi
    mov		dx, word ptr [rax+rbx+1]
    mov     word ptr [rbp+0x230-0x230+1], dx

    add		rbx, 2

.LmapAdjProc2_JMP_C:
    mov		r11, qword ptr [rip + mapAdjustmentProc2ReturnAddress]
    push	r11
    ret

.globl mapAdjustmentProc3V137
mapAdjustmentProc3V137:
    mov		dword ptr[rbp + 0x230 - 0x230], 0x000
    cmp     rbx, r12
    jz		.LmapAdjProc3_JMP_A
    lea     rdx, [rbp+0x230-0x1C0]
    cmp     rdi, 0x10
    cmovnb  rdx, [rbp+0x230-0x1C0]
    mov     r8, [rbp+0x230-0x1B0]
    lea     rcx, [rbp+0x230-0x200]

    mov		r11, qword ptr [rip + mapAdjustmentProc3ReturnAddress1]
    push	r11
    ret

.LmapAdjProc3_JMP_A:
    mov		r11, qword ptr [rip + mapAdjustmentProc3ReturnAddress2]
    push	r11
    ret

.globl mapAdjustmentProc4V137
mapAdjustmentProc4V137:
    lea     rax, [rbp+0x230-0x1A0]
    cmp     r13, 0x10
    cmovnb  rax, rsi

    cmp		byte ptr[rax+rcx], ESCAPE_SEQ_1
    jz		.LmapAdjProc4_JMP_A
    cmp		byte ptr[rax+rcx], ESCAPE_SEQ_2
    jz		.LmapAdjProc4_JMP_B
    cmp		byte ptr[rax+rcx], ESCAPE_SEQ_3
    jz		.LmapAdjProc4_JMP_C
    cmp		byte ptr[rax+rcx], ESCAPE_SEQ_4
    jz		.LmapAdjProc4_JMP_D
    movzx   eax, byte ptr [rax+rcx]
    jmp		.LmapAdjProc4_JMP_E

.LmapAdjProc4_JMP_A:
    movzx	eax, word ptr[rax+rcx + 1]
    jmp		.LmapAdjProc4_JMP_F

.LmapAdjProc4_JMP_B:
    movzx	eax, word ptr[rax+rcx + 1]
    sub		eax, SHIFT_2
    jmp		.LmapAdjProc4_JMP_F

.LmapAdjProc4_JMP_C:
    movzx	eax, word ptr[rax+rcx + 1]
    add		eax, SHIFT_3
    jmp		.LmapAdjProc4_JMP_F

.LmapAdjProc4_JMP_D:
    movzx	eax, word ptr[rax+rcx + 1]
    add		eax, SHIFT_4

.LmapAdjProc4_JMP_F:
    add		rcx, 2
    cmp		rcx, MAP_LIMIT
    ja		.LmapAdjProc4_JMP_G

    cmp		eax, NO_FONT
    ja		.LmapAdjProc4_JMP_E
.LmapAdjProc4_JMP_G:
    mov		eax, NOT_DEF
    movzx	eax, ax

.LmapAdjProc4_JMP_E:

    mov		r11, qword ptr [rip + mapAdjustmentProc4ReturnAddress]
    push	r11
    ret
