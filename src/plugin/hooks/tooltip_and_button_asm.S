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

.extern tooltipAndButtonProc1ReturnAddress
.extern tooltipAndButtonProc1CallAddress
.extern tooltipAndButtonProc2ReturnAddress
.extern tooltipAndButtonProc3ReturnAddress
.extern tooltipAndButtonProc4ReturnAddress1
.extern tooltipAndButtonProc4ReturnAddress2
.extern tooltipAndButtonProc4ReturnAddress3
.extern tooltipAndButtonProc5ReturnAddress1
.extern tooltipAndButtonProc5ReturnAddress2
.extern tooltipAndButtonProc7ReturnAddress1
.extern tooltipAndButtonProc7ReturnAddress2
.extern tooltipAndButtonProc8ReturnAddress1
.extern tooltipAndButtonProc9ReturnAddress1
.extern tooltipAndButtonProc9ReturnAddress2
.extern tooltipAndButtonProc10ReturnAddress1
.extern tooltipAndButtonProc10BufferWidth

.data
.globl tooltipAndButtonProc2TmpCharacter
tooltipAndButtonProc2TmpCharacter:
    .long 0
.globl tooltipAndButtonProc2TmpCharacterAddress
tooltipAndButtonProc2TmpCharacterAddress:
    .quad 0
.globl tooltipAndButtonProc2TmpFlag
tooltipAndButtonProc2TmpFlag:
    .long 0

.text
.globl tooltipAndButtonProc1V137
tooltipAndButtonProc1V137:
    cmp		byte ptr [rax + rbx], ESCAPE_SEQ_1
    jz		.LttbProc1_JMP_A
    cmp		byte ptr [rax + rbx], ESCAPE_SEQ_2
    jz		.LttbProc1_JMP_A
    cmp		byte ptr [rax + rbx], ESCAPE_SEQ_3
    jz		.LttbProc1_JMP_A
    cmp		byte ptr [rax + rbx], ESCAPE_SEQ_4
    jz		.LttbProc1_JMP_A

    movzx	r8d, byte ptr[rax + rbx]
    mov     edx, 1
    lea     rcx, qword ptr [rsp + 0x2290 -0x2248]
    mov		dword ptr [rip + tooltipAndButtonProc2TmpFlag], 0x0
    mov		r11, qword ptr [rip + tooltipAndButtonProc1CallAddress]
    call	r11

    jmp		.LttbProc1_JMP_B
.LttbProc1_JMP_A:
    mov		dword ptr [rip + tooltipAndButtonProc2TmpFlag], 0x1


    lea		r8, qword ptr [rax + rbx]
    mov		qword ptr [rip + tooltipAndButtonProc2TmpCharacterAddress], r8
    movzx	r8d, byte ptr[rax + rbx]
    mov     edx, 3 # The memory is allocated 3 byte, but the first byte is copied 3 times.
    lea     rcx, qword ptr [rsp + 0x2290 - 0x2248]
    mov		r11, qword ptr [rip + tooltipAndButtonProc1CallAddress]
    call	r11

    # overwrite
    mov		rcx, qword ptr [rip + tooltipAndButtonProc2TmpCharacterAddress]
    mov		cx, word ptr [rcx+1]
    mov		word ptr [rax+1], cx
.LttbProc1_JMP_B:
    mov		r11, qword ptr [rip + tooltipAndButtonProc1ReturnAddress]
    push	r11
    ret

.globl tooltipAndButtonProc2V137
tooltipAndButtonProc2V137:
    mov     r9d, r14d

    cmp		byte ptr[r9+rax], ESCAPE_SEQ_1
    jz		.LttbProc2_JMP_A
    cmp		byte ptr[r9+rax], ESCAPE_SEQ_2
    jz		.LttbProc2_JMP_B
    cmp		byte ptr[r9+rax], ESCAPE_SEQ_3
    jz		.LttbProc2_JMP_C
    cmp		byte ptr[r9+rax], ESCAPE_SEQ_4
    jz		.LttbProc2_JMP_D
    jmp		.LttbProc2_JMP_E

.LttbProc2_JMP_A:
    movzx	eax, word ptr[r9+rax + 1]
    jmp		.LttbProc2_JMP_F

.LttbProc2_JMP_B:
    movzx	eax, word ptr[r9+rax + 1]
    sub		eax, SHIFT_2
    jmp		.LttbProc2_JMP_F

.LttbProc2_JMP_C:
    movzx	eax, word ptr[r9+rax + 1]
    add		eax, SHIFT_3
    jmp		.LttbProc2_JMP_F

.LttbProc2_JMP_D:
    movzx	eax, word ptr[r9+rax + 1]
    add		eax, SHIFT_4
    jmp		.LttbProc2_JMP_F

.LttbProc2_JMP_E:
    movzx   eax, byte ptr [r9+rax]
    jmp		.LttbProc2_JMP_G

.LttbProc2_JMP_F:
    movzx	eax, ax
    add		r9d,2

    cmp		eax, NO_FONT
    ja		.LttbProc2_JMP_G
    mov		eax, NOT_DEF

.LttbProc2_JMP_G:
    mov		dword ptr [rip + tooltipAndButtonProc2TmpCharacter], eax
    # r11 is used as data; push return address first using rdx (not used as index here)
    mov     rdx, qword ptr [rip + tooltipAndButtonProc2ReturnAddress]
    push    rdx
    mov     r11, [rcx+rax*8]
    test    r11, r11
    ret

.globl tooltipAndButtonProc3V137
tooltipAndButtonProc3V137:
    mov     ecx, r15d
    movss   xmm11, dword ptr [rdx+0x848]

    cmp		byte ptr[rcx+rax], ESCAPE_SEQ_1
    jz		.LttbProc3_JMP_A
    cmp		byte ptr[rcx+rax], ESCAPE_SEQ_2
    jz		.LttbProc3_JMP_B
    cmp		byte ptr[rcx+rax], ESCAPE_SEQ_3
    jz		.LttbProc3_JMP_C
    cmp		byte ptr[rcx+rax], ESCAPE_SEQ_4
    jz		.LttbProc3_JMP_D
    jmp		.LttbProc3_JMP_E

.LttbProc3_JMP_A:
    movzx	eax, word ptr[rcx+rax + 1]
    jmp		.LttbProc3_JMP_F

.LttbProc3_JMP_B:
    movzx	eax, word ptr[rcx+rax + 1]
    sub		eax, SHIFT_2
    jmp		.LttbProc3_JMP_F

.LttbProc3_JMP_C:
    movzx	eax, word ptr[rcx+rax + 1]
    add		eax, SHIFT_3
    jmp		.LttbProc3_JMP_F

.LttbProc3_JMP_D:
    movzx	eax, word ptr[rcx+rax + 1]
    add		eax, SHIFT_4
    jmp		.LttbProc3_JMP_F

.LttbProc3_JMP_E:
    movzx   eax, byte ptr [rcx+rax]
    jmp		.LttbProc3_JMP_G

.LttbProc3_JMP_F:
    movzx	eax, ax
    add		r15d, 2
    cmp		eax, NO_FONT
    ja		.LttbProc3_JMP_G
    mov		eax, NOT_DEF

.LttbProc3_JMP_G:
    mov     r8, [rdx+rax*8]
    mov     [rbp+0x2190-0x2138], r8

    mov		r11, qword ptr [rip + tooltipAndButtonProc3ReturnAddress]
    push	r11
    ret

.globl tooltipAndButtonProc4V137
tooltipAndButtonProc4V137:
    cmp		word ptr [r11 + 6], 0
    jnz		.LttbProc4_JMP_A

    cmp		dword ptr [rbp+0x2190-0x21E0], 0
    jz		.LttbProc4_JMP_B

    jmp		.LttbProc4_JMP_C


.LttbProc4_JMP_A:
    cmp		dword ptr [rip + tooltipAndButtonProc2TmpCharacter], 0x0FF
    ja		.LttbProc4_JMP_Y

    mov		r11, qword ptr [rip + tooltipAndButtonProc4ReturnAddress1]
    push	r11
    ret


.LttbProc4_JMP_B:
    mov		r11, qword ptr [rip + tooltipAndButtonProc4ReturnAddress2]
    push	r11
    ret

.LttbProc4_JMP_Y:
    nop

.LttbProc4_JMP_C:
    mov     rax, [rbp + 0x2190 + 0x58]
    mov     eax, [rax]
    add     eax, eax
    xorps   xmm1, xmm1
    cvtsi2ss xmm1, rax
    addss   xmm1, xmm6
    mov     eax, [rsp + 0x2290 - 0x2220]
    xorps   xmm0, xmm0
    cvtsi2ss xmm0, rax
    comiss  xmm1, xmm0
    jb      .LttbProc4_JMP_B

    mov		r11, qword ptr [rip + tooltipAndButtonProc4ReturnAddress3]
    push	r11
    ret


.LttbProc4_JMP_X:
    nop
    mov		r11, qword ptr [rip + tooltipAndButtonProc4ReturnAddress2]
    push	r11
    ret


.globl tooltipAndButtonProc5V137
tooltipAndButtonProc5V137:
    lea     rcx, [r15+0x120]

    cmp		byte ptr [rbx+rbp], ESCAPE_SEQ_1
    jz		.LttbProc5_JMP_A
    cmp		byte ptr [rbx+rbp], ESCAPE_SEQ_2
    jz		.LttbProc5_JMP_B
    cmp		byte ptr [rbx+rbp], ESCAPE_SEQ_3
    jz		.LttbProc5_JMP_C
    cmp		byte ptr [rbx+rbp], ESCAPE_SEQ_4
    jz		.LttbProc5_JMP_D

    movzx   edx, byte ptr [rbx+rbp]
    jmp		.LttbProc5_JMP_G

.LttbProc5_JMP_A:
    movzx	edx, word ptr[rbx+rbp + 1]
    jmp		.LttbProc5_JMP_F

.LttbProc5_JMP_B:
    movzx	edx, word ptr[rbx+rbp + 1]
    sub		edx, SHIFT_2
    jmp		.LttbProc5_JMP_F

.LttbProc5_JMP_C:
    movzx	edx, word ptr[rbx+rbp + 1]
    add		edx, SHIFT_3
    jmp		.LttbProc5_JMP_F

.LttbProc5_JMP_D:
    movzx	edx, word ptr[rbx+rbp + 1]
    add		edx, SHIFT_4

.LttbProc5_JMP_F:
    movzx	edx, dx
    cmp		edx, NO_FONT
    ja		.LttbProc5_JMP_H
    mov		edx, NOT_DEF

.LttbProc5_JMP_H:
    add		rbx, 3
    add		edi, 3
    cmp		rbx, r12
    ja		.LttbProc5_JMP_J
    dec		rbx
    dec		edi

.LttbProc5_JMP_G:
    # r11 is used as data; push return address first using rax (not used as index here)
    mov     rax, qword ptr [rip + tooltipAndButtonProc5ReturnAddress1]
    push    rax
    mov     r11, [rcx+rdx*8]
    test    r11, r11
    ret

.LttbProc5_JMP_J:
    movd    xmm0, eax
    mov		r11, qword ptr [rip + tooltipAndButtonProc5ReturnAddress2]
    push	r11
    ret

.globl tooltipAndButtonProc7V137
tooltipAndButtonProc7V137:
    cmp		dword ptr [rip + tooltipAndButtonProc2TmpFlag], 1
    jnz		.LttbProc7_JMP_A

    add		r14d,2

.LttbProc7_JMP_A:
    inc		r14d
    cmp     r14d, dword ptr [rbp+0x2190-0x21B8]
    jl		.LttbProc7_JMP_B
    mov     r15d, dword ptr [rbp+0x2190+0x38]
    mov		r11, qword ptr [rip + tooltipAndButtonProc7ReturnAddress2]
    push	r11
    ret

.LttbProc7_JMP_B:
    mov		r11, qword ptr [rip + tooltipAndButtonProc7ReturnAddress1]
    push	r11
    ret
