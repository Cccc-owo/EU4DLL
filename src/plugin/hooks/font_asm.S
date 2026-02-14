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

.extern fontBufferHeapZeroClearReturnAddress
.extern fontBufferHeapZeroClearHeapJmpAddress
.extern fontBufferHeapZeroClearHeepAllocJmpAddress

.text
.globl fontBufferHeapZeroClear
fontBufferHeapZeroClear:
    # Load the value of the pointer variable (double indirection)
    mov		rcx, qword ptr [rip + fontBufferHeapZeroClearHeapJmpAddress]
    mov		rcx, qword ptr [rcx]
    mov		r8,rbx
    mov		edx,0x8
    mov		rax, qword ptr [rip + fontBufferHeapZeroClearHeepAllocJmpAddress]
    call	qword ptr [rax]
    test	rax,rax
    mov		r11, qword ptr [rip + fontBufferHeapZeroClearReturnAddress]
    push	r11
    ret
