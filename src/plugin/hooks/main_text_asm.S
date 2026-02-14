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

.extern mainTextProc1ReturnAddress
.extern mainTextProc2ReturnAddress
.extern mainTextProc2BufferAddress
.extern mainTextProc3ReturnAddress1
.extern mainTextProc3ReturnAddress2
.extern mainTextProc4ReturnAddress

.data
.globl mainTextProc2TmpCharacter
mainTextProc2TmpCharacter:
    .long 0

.text
.globl mainTextProc1V137
mainTextProc1V137:
    cmp		byte ptr [rcx+r9], ESCAPE_SEQ_1
    jz		.LmainTextProc1_JMP_A
    cmp		byte ptr [rcx+r9], ESCAPE_SEQ_2
    jz		.LmainTextProc1_JMP_B
    cmp		byte ptr [rcx+r9], ESCAPE_SEQ_3
    jz		.LmainTextProc1_JMP_C
    cmp		byte ptr [rcx+r9], ESCAPE_SEQ_4
    jz		.LmainTextProc1_JMP_D
    movzx   r8d, byte ptr [rcx+r9]
    jmp		.LmainTextProc1_JMP_E

.LmainTextProc1_JMP_A:
    movzx	r8d, word ptr[rcx+r9 + 1]
    jmp		.LmainTextProc1_JMP_F

.LmainTextProc1_JMP_B:
    movzx	r8d, word ptr[rcx+r9 + 1]
    sub		r8d, SHIFT_2
    jmp		.LmainTextProc1_JMP_F

.LmainTextProc1_JMP_C:
    movzx	r8d, word ptr[rcx+r9 + 1]
    add		r8d, SHIFT_3
    jmp		.LmainTextProc1_JMP_F

.LmainTextProc1_JMP_D:
    movzx	r8d, word ptr[rcx+r9 + 1]
    add		r8d, SHIFT_4

.LmainTextProc1_JMP_F:
    movzx	r8d, r8w
    add		r15d, 2
    cmp		r8d, NO_FONT

    ja		.LmainTextProc1_JMP_E
    mov		r8d, NOT_DEF
.LmainTextProc1_JMP_E:
    movss   xmm3, dword ptr [r14+0x968]
    mov     rdx, qword ptr [r14+r8*8+0x120]

    mov		r11, qword ptr [rip + mainTextProc1ReturnAddress]
    push	r11
    ret

.globl mainTextProc2V137
mainTextProc2V137:
    movsxd  r9, edi
    mov     rdx, [rbp+0x2340-0x2348]
    add     r9, rdx
    movsxd  rcx, esi
    movzx   eax, byte ptr [r9]
    mov     r11, qword ptr [rip + mainTextProc2BufferAddress]
    mov     byte ptr[rcx+r11], al
    inc     esi
    inc		rcx

    cmp		al, ESCAPE_SEQ_1
    jz		.LmainTextProc2_JMP_A
    cmp		al, ESCAPE_SEQ_2
    jz		.LmainTextProc2_JMP_B
    cmp		al, ESCAPE_SEQ_3
    jz		.LmainTextProc2_JMP_C
    cmp		al, ESCAPE_SEQ_4
    jz		.LmainTextProc2_JMP_D
    jmp		.LmainTextProc2_JMP_E

.LmainTextProc2_JMP_A:
    movzx	eax, word ptr[r9+1]
    mov		word ptr [rcx+r11], ax
    jmp		.LmainTextProc2_JMP_F

.LmainTextProc2_JMP_B:
    movzx	eax, word ptr[r9+1]
    mov		word ptr [rcx+r11], ax
    sub		eax, SHIFT_2
    jmp		.LmainTextProc2_JMP_F

.LmainTextProc2_JMP_C:
    movzx	eax, word ptr [r9+1]
    mov		word ptr [rcx+r11], ax
    add		eax, SHIFT_3
    jmp		.LmainTextProc2_JMP_F

.LmainTextProc2_JMP_D:
    movzx	eax, word ptr [r9+1]
    mov		word ptr [rcx+r11], ax
    add		eax, SHIFT_4

.LmainTextProc2_JMP_F:
    movzx	eax, ax
    add		esi, 2
    add		rcx,2
    cmp		eax, NO_FONT

    ja		.LmainTextProc2_JMP_G
    mov		eax, NOT_DEF

.LmainTextProc2_JMP_G:
    add		edi, 2
    add		r9, 2
.LmainTextProc2_JMP_E:

    mov		dword ptr [rip + mainTextProc2TmpCharacter], eax

    # r11 is used as data in this proc; use rdx as scratch for return address
    mov		rdx, qword ptr [rip + mainTextProc2ReturnAddress]
    push	rdx
    ret

.globl mainTextProc3
mainTextProc3:
    cmp		word ptr [rcx+6],0
    jnz		.LmainTextProc3_JMP_A
    jmp		.LmainTextProc3_JMP_B

.LmainTextProc3_JMP_A:
    cmp		dword ptr [rip + mainTextProc2TmpCharacter], 0x0FF
    ja		.LmainTextProc3_JMP_B

    mov		r11, qword ptr [rip + mainTextProc3ReturnAddress2]
    push	r11
    ret

.LmainTextProc3_JMP_B:
    lea     eax, dword ptr [rbx+rbx]
    movd    xmm1, eax

    mov		r11, qword ptr [rip + mainTextProc3ReturnAddress1]
    push	r11
    ret


.globl mainTextProc4V137
mainTextProc4V137:
    # check code point saved proc1
    cmp		dword ptr [rip + mainTextProc2TmpCharacter], 0x0FF
    ja		.LmainTextProc4_JMP_A

    movzx	eax, byte ptr[r9]
    jmp		.LmainTextProc4_JMP_B

.LmainTextProc4_JMP_A:
    mov		eax, dword ptr [rip + mainTextProc2TmpCharacter]

.LmainTextProc4_JMP_B:
    mov     rcx, [r14+rax*8+0x120]
    mov     [rbp+0x2340-0x2340], rcx
    test	rcx,rcx

    mov		r11, qword ptr [rip + mainTextProc4ReturnAddress]
    push	r11
    ret
