#ifndef ISR_H
#define ISR_H

#include <stdint.h>

/* The frame isr.S builds. The CPU pushes the tail (rip through ss); the stub
 * pushes the vector and error code, then the general purpose registers. */
struct registers {
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
	uint64_t int_no, err_code;
	uint64_t rip, cs, rflags, rsp, ss;
};

typedef void (*isr_handler_t)(struct registers *regs);

/* Installs the IDT and remaps the PICs. Leaves interrupts disabled and every
 * IRQ masked; callers unmask the lines they have handlers for and run sti. */
void isr_init(void);

/* Vectors 0-31 are exceptions, 32-47 the remapped IRQs. Registering an
 * exception handler replaces the default panic. */
void isr_register_handler(uint8_t vector, isr_handler_t handler);

/* Enables interrupts. */
void isr_enable(void);

const char *isr_exception_name(uint64_t vector);

#endif
