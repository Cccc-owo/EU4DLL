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

.extern eventDialogProc1ReturnAddress
.extern eventDialogProc2ReturnAddress1
.extern eventDialogProc2ReturnAddress2
.extern eventDialogProc3ReturnAddress

.data
.globl eventDialogProc1Flag
eventDialogProc1Flag:
    .quad 0

.text
.globl eventDialogProc1V137
eventDialogProc1V137:
    cmp		byte ptr [rax+rdx], ESCAPE_SEQ_1
    jz		.LeventDialogProc1_JMP_A
    cmp		byte ptr [rax+rdx], ESCAPE_SEQ_2
    jz		.LeventDialogProc1_JMP_B
    cmp		byte ptr [rax+rdx], ESCAPE_SEQ_3
    jz		.LeventDialogProc1_JMP_C
    cmp		byte ptr [rax+rdx], ESCAPE_SEQ_4
    jz		.LeventDialogProc1_JMP_D

    mov		qword ptr [rip + eventDialogProc1Flag], 0x0
    movzx	eax, byte ptr [rax+rdx]
    jmp		.LeventDialogProc1_JMP_E

.LeventDialogProc1_JMP_A:
    movzx	eax, word ptr [rax+rdx + 1]
    jmp		.LeventDialogProc1_JMP_G

.LeventDialogProc1_JMP_B:
    movzx	eax, word ptr [rax+rdx + 1]
    sub		eax, SHIFT_2
    jmp		.LeventDialogProc1_JMP_G

.LeventDialogProc1_JMP_C:
    movzx	eax, word ptr [rax+rdx + 1]
    add		eax, SHIFT_3
    jmp		.LeventDialogProc1_JMP_G

.LeventDialogProc1_JMP_D:
    movzx	eax, word ptr [rax+rdx + 1]
    add		eax, SHIFT_4

.LeventDialogProc1_JMP_G:
    movzx	eax, ax
    cmp		eax, NO_FONT
    ja		.LeventDialogProc1_JMP_F
    mov		eax, NOT_DEF

.LeventDialogProc1_JMP_F:
    mov		qword ptr [rip + eventDialogProc1Flag], 0x1

.LeventDialogProc1_JMP_E:
    # r11 is used as data; push return address first using rdx as scratch (rdx not needed after)
    mov     rdx, qword ptr [rip + eventDialogProc1ReturnAddress]
    push    rdx
    mov     r11, qword ptr [r13+rax*8+0x120]
    movss   xmm1, dword ptr [r13+0x968]
    test    r11, r11
    ret

.globl eventDialogProc2V137
eventDialogProc2V137:
    cvtdq2ps	xmm0, xmm0
    mulss		xmm0, xmm1
    ucomiss		xmm0, xmm8

    cmp			qword ptr [rip + eventDialogProc1Flag],0x1
    jz			.LeventDialogProc2_JMP_B
    jp			.LeventDialogProc2_JMP_B
    jnz			.LeventDialogProc2_JMP_B

.LeventDialogProc2_JMP_A:
    mov		r11, qword ptr [rip + eventDialogProc2ReturnAddress2]
    push		r11
    ret

.LeventDialogProc2_JMP_B:
    mov		r11, qword ptr [rip + eventDialogProc2ReturnAddress1]
    push		r11
    ret

.globl eventDialogProc3V137
eventDialogProc3V137:
    cmp		qword ptr [rip + eventDialogProc1Flag], 1
    jnz		.LeventDialogProc3_JMP_A
    add		edi,2

.LeventDialogProc3_JMP_A:
    inc		edi
    mov     r9d, [rbx+0x10]
    cmp     edi, r9d
    mov     ecx, [rbp+0x1060+0x10]
    mov		r11, qword ptr [rip + eventDialogProc3ReturnAddress]
    push	r11
    ret
