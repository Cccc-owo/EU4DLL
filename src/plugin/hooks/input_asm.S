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

.extern inputProc1ReturnAddress1
.extern inputProc1ReturnAddress2
.extern inputProc1CallAddress
.extern inputProc2ReturnAddress

.data
.globl inputProc1Var1
inputProc1Var1:
    .byte 03, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
.globl inputProc1Tmp
inputProc1Tmp:
    .quad 0
.globl inputProc2Tmp
inputProc2Tmp:
    .quad 0
.globl inputProc2Tmp2
inputProc2Tmp2:
    .quad 0

.text
.globl inputProc1V137
inputProc1V137:
    mov		eax, dword ptr [rbp + 0x60 - 0xA4]
    cmp		ah, 0
    jnz		.LinputProc1_JMP_A
    xor		bl, bl

    cmp		al, 0x80
    jnb		.LinputProc1_JMP_X
    mov		ebx, eax
    jmp     .LinputProc1_JMP_Y

.LinputProc1_JMP_X:
    cmp		al, 0xE0
    jnb		.LinputProc1_JMP_Y
    movzx	ebx, al
    movzx	ebx, byte ptr [rbp + 0x60 - 0xA4 + 1]
    and		al, 0x3F
    shl		bl, 6
    or		bl, al

.LinputProc1_JMP_Y:
    push    rax
    mov     rax, qword ptr [rip + inputProc1ReturnAddress1]
    xchg    qword ptr [rsp], rax
    ret

.LinputProc1_JMP_A:
    lea		rcx,[rbp + 0x60 - 0xA4]
    mov		r11, qword ptr [rip + inputProc1CallAddress]
    call	r11
    mov		qword ptr [rip + inputProc2Tmp], rax
    mov		qword ptr [rip + inputProc1Tmp], rdi
    xor		rdi,rdi

.LinputProc1_JMP_B:
    mov     rax, [r15]
    xor     r9d, r9d
    mov     r8d, [rbp+0x60-0x9C]
    mov     edx, 0x303
    mov     rcx, r15
    call    qword ptr [rax+0x20]

    mov		rbx, qword ptr [rip + inputProc2Tmp]
    mov		bl, byte ptr [rbx + rdi]

    cmp		bl,0
    jz		.LinputProc1_JMP_C

    mov		dword ptr [r14+0x44] , 2

    xorps   xmm0, xmm0
    mov     [rsp+0x160-0x100], bl
    xorps   xmm1, xmm1
    mov     qword ptr [rsp+0x160-0xEC], 0
    movdqu  [rsp+0x160-0x110], xmm0
    mov     [rsp+0x160-0xE4], r12d
    movdqu  [rsp+0x160-0xFC], xmm1
    mov     DWORD ptr [rbp+0x60-0xC0], 2
    movdqa  xmm0, xmmword ptr [rip + inputProc1Var1]
    lea     rdx, [rsp+0x160-0x110]
    mov     rax, [r14]
    mov     rcx, r14
    mov     [rbp+0x60-0xC8], r12d
    movdqu  [rbp+0x60-0xE0+8], xmm0
    mov     qword ptr [rbp+0x60-0xE0], r12
    mov     [rbp+0x60-0xBC], r12w
    mov     byte ptr [rbp+0x60-0xBA], 0
    call    qword ptr [rax+0x18]

    inc		rdi
    jmp		.LinputProc1_JMP_B

.LinputProc1_JMP_C:
    mov		rdi, qword ptr [rip + inputProc1Tmp]

    push    rax
    mov     rax, qword ptr [rip + inputProc1ReturnAddress2]
    xchg    qword ptr [rsp], rax
    ret

.globl inputProc2V137
inputProc2V137:
    mov		qword ptr [rip + inputProc2Tmp2], rsi
    xor		rsi,rsi

    xor     ecx, ecx
    mov     [rsp+0x48-0x28], rcx
    mov     qword ptr [rsp+0x48-0x10], 0xF
    mov     [rsp+0x48-0x18], rcx
    cmp     [rdi+0x80], rcx


    mov		rcx, qword ptr [rdi + 0x40]
    cmp		rcx, 0x10
    lea		rcx, [rdi + 0x30]
    jbe		.LinputProc2_JMP_A
    mov		rcx, [rcx]

.LinputProc2_JMP_A:
    movsxd	rax, dword ptr [rdi + 0x54]
    sub		rax, 3
    js		.LinputProc2_JMP_C
    mov		al, byte ptr [rcx + rax]
    cmp		al, ESCAPE_SEQ_1
    jz		.LinputProc2_JMP_B
    cmp		al, ESCAPE_SEQ_2
    jz		.LinputProc2_JMP_B
    cmp		al, ESCAPE_SEQ_3
    jz		.LinputProc2_JMP_B
    cmp		al, ESCAPE_SEQ_4
    jnz		.LinputProc2_JMP_C

.LinputProc2_JMP_B:
    mov		rsi, 2

.LinputProc2_JMP_C:
    mov		rax, qword ptr [rdi]
    mov		rcx, rdi
    cmp		qword ptr [rdi+0x80] ,0
    jz		.LinputProc2_JMP_D
    call	qword ptr [rax+0x140]
    jmp		.LinputProc2_JMP_E

.LinputProc2_JMP_D:
    call	qword ptr [rax+0x138]

.LinputProc2_JMP_E:
    cmp		rsi, 0
    jz      .LinputProc2_JMP_F
    dec		rsi
    jmp		.LinputProc2_JMP_C

.LinputProc2_JMP_F:
    mov		rsi, qword ptr [rip + inputProc2Tmp2]

    push    rax
    mov     rax, qword ptr [rip + inputProc2ReturnAddress]
    xchg    qword ptr [rsp], rax
    ret
