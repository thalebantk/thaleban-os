/* Interrupt entry stubs. Generated as a flat list rather than a loop so the
 * error-code exceptions can be distinguished at assembly time: the CPU pushes
 * an error code for some vectors and not others, and every handler needs to
 * see the same stack frame. */

	.section .text

	.align 8
	.globl isr0
isr0:
	pushq $0
	pushq $0
	jmp isr_common

	.align 8
	.globl isr1
isr1:
	pushq $0
	pushq $1
	jmp isr_common

	.align 8
	.globl isr2
isr2:
	pushq $0
	pushq $2
	jmp isr_common

	.align 8
	.globl isr3
isr3:
	pushq $0
	pushq $3
	jmp isr_common

	.align 8
	.globl isr4
isr4:
	pushq $0
	pushq $4
	jmp isr_common

	.align 8
	.globl isr5
isr5:
	pushq $0
	pushq $5
	jmp isr_common

	.align 8
	.globl isr6
isr6:
	pushq $0
	pushq $6
	jmp isr_common

	.align 8
	.globl isr7
isr7:
	pushq $0
	pushq $7
	jmp isr_common

	.align 8
	.globl isr8
isr8:
	pushq $8
	jmp isr_common

	.align 8
	.globl isr9
isr9:
	pushq $0
	pushq $9
	jmp isr_common

	.align 8
	.globl isr10
isr10:
	pushq $10
	jmp isr_common

	.align 8
	.globl isr11
isr11:
	pushq $11
	jmp isr_common

	.align 8
	.globl isr12
isr12:
	pushq $12
	jmp isr_common

	.align 8
	.globl isr13
isr13:
	pushq $13
	jmp isr_common

	.align 8
	.globl isr14
isr14:
	pushq $14
	jmp isr_common

	.align 8
	.globl isr15
isr15:
	pushq $0
	pushq $15
	jmp isr_common

	.align 8
	.globl isr16
isr16:
	pushq $0
	pushq $16
	jmp isr_common

	.align 8
	.globl isr17
isr17:
	pushq $17
	jmp isr_common

	.align 8
	.globl isr18
isr18:
	pushq $0
	pushq $18
	jmp isr_common

	.align 8
	.globl isr19
isr19:
	pushq $0
	pushq $19
	jmp isr_common

	.align 8
	.globl isr20
isr20:
	pushq $0
	pushq $20
	jmp isr_common

	.align 8
	.globl isr21
isr21:
	pushq $21
	jmp isr_common

	.align 8
	.globl isr22
isr22:
	pushq $0
	pushq $22
	jmp isr_common

	.align 8
	.globl isr23
isr23:
	pushq $0
	pushq $23
	jmp isr_common

	.align 8
	.globl isr24
isr24:
	pushq $0
	pushq $24
	jmp isr_common

	.align 8
	.globl isr25
isr25:
	pushq $0
	pushq $25
	jmp isr_common

	.align 8
	.globl isr26
isr26:
	pushq $0
	pushq $26
	jmp isr_common

	.align 8
	.globl isr27
isr27:
	pushq $0
	pushq $27
	jmp isr_common

	.align 8
	.globl isr28
isr28:
	pushq $0
	pushq $28
	jmp isr_common

	.align 8
	.globl isr29
isr29:
	pushq $29
	jmp isr_common

	.align 8
	.globl isr30
isr30:
	pushq $30
	jmp isr_common

	.align 8
	.globl isr31
isr31:
	pushq $0
	pushq $31
	jmp isr_common

	.align 8
	.globl isr32
isr32:
	pushq $0
	pushq $32
	jmp isr_common

	.align 8
	.globl isr33
isr33:
	pushq $0
	pushq $33
	jmp isr_common

	.align 8
	.globl isr34
isr34:
	pushq $0
	pushq $34
	jmp isr_common

	.align 8
	.globl isr35
isr35:
	pushq $0
	pushq $35
	jmp isr_common

	.align 8
	.globl isr36
isr36:
	pushq $0
	pushq $36
	jmp isr_common

	.align 8
	.globl isr37
isr37:
	pushq $0
	pushq $37
	jmp isr_common

	.align 8
	.globl isr38
isr38:
	pushq $0
	pushq $38
	jmp isr_common

	.align 8
	.globl isr39
isr39:
	pushq $0
	pushq $39
	jmp isr_common

	.align 8
	.globl isr40
isr40:
	pushq $0
	pushq $40
	jmp isr_common

	.align 8
	.globl isr41
isr41:
	pushq $0
	pushq $41
	jmp isr_common

	.align 8
	.globl isr42
isr42:
	pushq $0
	pushq $42
	jmp isr_common

	.align 8
	.globl isr43
isr43:
	pushq $0
	pushq $43
	jmp isr_common

	.align 8
	.globl isr44
isr44:
	pushq $0
	pushq $44
	jmp isr_common

	.align 8
	.globl isr45
isr45:
	pushq $0
	pushq $45
	jmp isr_common

	.align 8
	.globl isr46
isr46:
	pushq $0
	pushq $46
	jmp isr_common

	.align 8
	.globl isr47
isr47:
	pushq $0
	pushq $47
	jmp isr_common

	.align 16
isr_common:
	/* Pushed last-to-first so the frame matches struct registers, whose
	 * first member sits at the lowest address. */
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rsi
	pushq %rdi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	/* The CPU aligns RSP to 16 before pushing its frame; 40 bytes of CPU
	 * frame plus 16 of vector/error plus 120 of registers brings it back
	 * to a 16-byte boundary, which is what the ABI wants at a call. */
	movq %rsp, %rdi
	call isr_handler

	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rdi
	popq %rsi
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax

	/* Discard the vector and error code the stub pushed. */
	addq $16, %rsp
	iretq

	.section .rodata
	.align 8
	.globl isr_stub_table
isr_stub_table:
	.quad isr0
	.quad isr1
	.quad isr2
	.quad isr3
	.quad isr4
	.quad isr5
	.quad isr6
	.quad isr7
	.quad isr8
	.quad isr9
	.quad isr10
	.quad isr11
	.quad isr12
	.quad isr13
	.quad isr14
	.quad isr15
	.quad isr16
	.quad isr17
	.quad isr18
	.quad isr19
	.quad isr20
	.quad isr21
	.quad isr22
	.quad isr23
	.quad isr24
	.quad isr25
	.quad isr26
	.quad isr27
	.quad isr28
	.quad isr29
	.quad isr30
	.quad isr31
	.quad isr32
	.quad isr33
	.quad isr34
	.quad isr35
	.quad isr36
	.quad isr37
	.quad isr38
	.quad isr39
	.quad isr40
	.quad isr41
	.quad isr42
	.quad isr43
	.quad isr44
	.quad isr45
	.quad isr46
	.quad isr47

	.section .note.GNU-stack,"",@progbits
