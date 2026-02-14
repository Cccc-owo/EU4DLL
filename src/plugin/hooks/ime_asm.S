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

.extern imeProc1ReturnAddress1
.extern imeProc1ReturnAddress2
.extern imeProc1CallAddress
.extern imeProc2ReturnAddress1
.extern imeProc2ReturnAddress2
.extern imeProc2CallAddress
.extern rectAddress
.extern imeProc3ReturnAddress
.extern imeProc3CallAddress1
.extern imeProc3CallAddress2
.extern imeProc3CallAddress3
.extern imeProc3CallAddress4
.extern imeProc3CallAddress5

.data
.globl aSdl_windows_no
aSdl_windows_no:
    .ascii "SDL_WINDOWS_NO_CLOSE_ON_ALT_F4"

.text
.globl imeProc1V137
imeProc1V137:
    mov		edx, edi
    mov		rcx, r12
    mov		r11, qword ptr [rip + imeProc1CallAddress]
    call	r11
    test	eax,eax
    jnz		.LimeProc1_JMP_A

    mov		r11, qword ptr [rip + imeProc1ReturnAddress1]
    push	r11
    ret

.LimeProc1_JMP_A:
    mov		r11, qword ptr [rip + imeProc1ReturnAddress2]
    push	r11
    ret


.globl imeProc2
imeProc2:
    sub		edx, 0xBC
    jz		.LimeProc2_JMP_A

    mov		r11, qword ptr [rip + imeProc2ReturnAddress2]
    push	r11
    ret

.LimeProc2_JMP_A:
    mov		rcx, qword ptr [rip + rectAddress]
    mov		r11, qword ptr [rip + imeProc2CallAddress]
    call	r11

    mov		r11, qword ptr [rip + imeProc2ReturnAddress1]
    push	r11
    ret

.globl imeProc3V137
imeProc3V137:
    mov		rcx,qword ptr [rbp + 0xD0 + 0x28]
    mov		rdx, rsi
    mov		r11, qword ptr [rip + imeProc3CallAddress1]
    call	r11
    xor		ecx, ecx
    mov		edi, eax
    mov		r11, qword ptr [rip + imeProc3CallAddress2]
    call	r11
    cmp		byte ptr [rax + 0xE2], r14b
    jz      .LimeProc3_JMP_A
    cmp		byte ptr [rax + 0xE6], r14b
    jnz		.LimeProc3_JMP_B

.LimeProc3_JMP_A:
    cmp		edi, 0x3D
    jnz		.LimeProc3_JMP_B
    lea		rcx, [rip + aSdl_windows_no]
    mov		r11, qword ptr [rip + imeProc3CallAddress3]
    call	r11
    test	rax, rax
    jz		.LimeProc3_JMP_C
    cmp		byte ptr [rax], 0x30
    jnz		.LimeProc3_JMP_E

.LimeProc3_JMP_C:
    mov		rcx, qword ptr [rbx]
    xor		r9d, r9d
    xor		r8d, r8d
    mov		dl, 0xE
    mov		r11, qword ptr [rip + imeProc3CallAddress4]
    call	r11
    jmp     .LimeProc3_JMP_E

.LimeProc3_JMP_B:
    test	edi, edi
    jz		.LimeProc3_JMP_D

.LimeProc3_JMP_E:
    cmp		rsi,229
    jz		.LimeProc3_JMP_D
    mov		edx, edi
    movzx	ecx, r14b
    mov		r11, qword ptr [rip + imeProc3CallAddress5]
    call    r11

.LimeProc3_JMP_D:
    mov		r11, qword ptr [rip + imeProc3ReturnAddress]
    push	r11
    ret
