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

.extern fileSaveProc1ReturnAddress
.extern fileSaveProc2CallAddress
.extern fileSaveProc2ReturnAddress
.extern fileSaveProc3ReturnAddress
.extern fileSaveProc3CallAddress
.extern fileSaveProc3CallAddress2
.extern fileSaveProc4CallAddress
.extern fileSaveProc4MarkerAddress
.extern fileSaveProc4ReturnAddress
.extern fileSaveProc5CallAddress
.extern fileSaveProc5MarkerAddress
.extern fileSaveProc5ReturnAddress
.extern fileSaveProc6CallAddress
.extern fileSaveProc6MarkerAddress
.extern fileSaveProc6ReturnAddress
.extern fileSaveProc7CallAddress
.extern fileSaveProc7ReturnAddress
.extern fileSaveProc9CallAddress
.extern fileSaveProc9ReturnAddress
.extern fileSaveProc10ReturnAddress

.data
.globl fileSaveProc3Tmp
fileSaveProc3Tmp:
    .quad 0
.globl fileSaveProc3Message
fileSaveProc3Message:
    .asciz "save_game_title"
.globl fileSaveProc9Text
fileSaveProc9Text:
    .asciz "CONFIRMLOADSAVETEXT"
.globl fileSaveProc10Text
fileSaveProc10Text:
    .asciz "SAVEGAME_WARNING_OLD_VERSION_DESC"

.text
.globl fileSaveProc1
fileSaveProc1:
    mov		eax, dword ptr [rcx + 0x10]
    test	eax, eax

    mov		r11, qword ptr [rip + fileSaveProc1ReturnAddress]
    push	r11
    ret

.globl fileSaveProc2V137
fileSaveProc2V137:
    mov		rcx, rax
    mov		r11, qword ptr [rip + fileSaveProc2CallAddress]
    call	r11

    cmp		qword ptr [rax+0x18], 0x10
    jb		.LfileSaveProc2_JMP_A
    mov		rax, [rax]

.LfileSaveProc2_JMP_A:

    mov     qword ptr [rbp+0x57-0x80], r12
    mov     qword ptr [rbp+0x57-0x70], r12
    mov     qword ptr [rbp+0x57-0x68], 0xF

    mov		r11, qword ptr [rip + fileSaveProc2ReturnAddress]
    push	r11
    ret

.globl fileSaveProc3V137
fileSaveProc3V137:
    lea     rcx, [rsi+0x240]
    mov		r11, qword ptr [rip + fileSaveProc3CallAddress]
    call	r11
    mov		rdx, rax

    xor     r8d, r8d
    mov     rcx, rbp
    mov		r11, qword ptr [rip + fileSaveProc3CallAddress2]
    call	r11

    mov		r11, qword ptr [rip + fileSaveProc3ReturnAddress]
    push	r11
    ret

.globl fileSaveProc5V137
fileSaveProc5V137:
    mov     rcx, rdx
    mov		r11, qword ptr [rip + fileSaveProc5CallAddress]
    call	r11
    mov		rdx, rax
    mov     r8, [rdx+0x10]

    cmp     qword ptr [rdx+0x18], 0x10
    jb		.LfileSaveProc5_JMP_A
    mov     rdx, [rdx]

.LfileSaveProc5_JMP_A:
    lea     rcx, [rsp+0x478-0x398]

    mov		r11, qword ptr [rip + fileSaveProc5ReturnAddress]
    push	r11
    ret

.globl fileSaveProc6V137
fileSaveProc6V137:
    lea     rdx, [rbp+0x2E0-0x240]
    mov		rcx, rdx
    mov		r11, qword ptr [rip + fileSaveProc6CallAddress]
    call	r11
    mov		rdx, rax
    mov     r8, [rdx+0x10]
    cmp     qword ptr [rdx+0x18], 0x10
    jb		.LfileSaveProc6_JMP_A
    mov     rdx, [rdx]
.LfileSaveProc6_JMP_A:
    lea     rcx, [rsp+0x3E0-0x390]

    mov		r11, qword ptr [rip + fileSaveProc6ReturnAddress]
    push	r11
    ret

.globl fileSaveProc6V137E
fileSaveProc6V137E:
    lea     rdx, [rbp+0x2E0-0x240]
    mov		rcx, rdx
    mov		r11, qword ptr [rip + fileSaveProc6CallAddress]
    call	r11
    mov		rdx, rax
    mov     r8, [rdx+0x10]
    cmp     qword ptr [rdx+0x18], 0x10
    jb		.LfileSaveProc6E_JMP_A
    mov     rdx, [rdx]
.LfileSaveProc6E_JMP_A:
    lea     rcx, [rsp+0x3E0-0x388]

    mov		r11, qword ptr [rip + fileSaveProc6ReturnAddress]
    push	r11
    ret

.globl fileSaveProc6V137G
fileSaveProc6V137G:
    lea     rdx, [rsp+0x560-0x510]
    mov     rcx, rdx
    mov     r11, qword ptr [rip + fileSaveProc6CallAddress]
    call    r11
    mov     rdx, rax
    mov     r8, [rdx+0x10]
    cmp     qword ptr [rdx+0x18], 0x10
    jb      .LfileSaveProc6G_JMP_A
    mov     rdx, [rdx]
.LfileSaveProc6G_JMP_A:
    lea     rcx, [rbp+0x460-0x4E0]

    mov     r11, qword ptr [rip + fileSaveProc6ReturnAddress]
    push    r11
    ret

.globl fileSaveProc7V137
fileSaveProc7V137:

    lea     rdx, [rsp+0x48-0x28]
    mov		rcx, rdx
    mov		r11, qword ptr [rip + fileSaveProc7CallAddress]
    call	r11
    mov		rdx, rax
    mov     r8, [rdx+0x10]

    lea		rcx, [rdi+0xC8]
    mov     rax, [rcx]

    mov		r11, qword ptr [rip + fileSaveProc7ReturnAddress]
    push	r11
    ret

.globl fileSaveProc9
fileSaveProc9:

    mov     rcx, rax
    mov		r11, qword ptr [rip + fileSaveProc9CallAddress]
    call	r11

    mov     r9d, 1
    lea     r8, [rbp+0x57-0x80]
    lea     rdx, [rip + fileSaveProc9Text]
    lea     rcx, [rbp+0x57-0xC0]

    mov		r11, qword ptr [rip + fileSaveProc9ReturnAddress]
    push	r11
    ret

.globl fileSaveProc10
fileSaveProc10:

    mov     rcx, rax
    mov		r11, qword ptr [rip + fileSaveProc9CallAddress]
    call	r11

    mov     r9d, 1
    lea     r8, [rbp+0x30-0x80]
    lea     rdx, [rip + fileSaveProc10Text]
    lea     rcx, [rsp+0x130-0xC0]

    mov		r11, qword ptr [rip + fileSaveProc10ReturnAddress]
    push	r11
    ret
