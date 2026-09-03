#include <stddef.h>

#include <console/console.h>
#include <cpu/idt.h>
#include <cpu/isr.h>
#include <drivers/pic.h>
#include <klib/kprintf.h>

#define EXCEPTION_COUNT 32
#define IRQ_COUNT       16
#define VECTOR_COUNT    (EXCEPTION_COUNT + IRQ_COUNT)

static isr_handler_t handlers[VECTOR_COUNT];

static const char *const exception_names[EXCEPTION_COUNT] = {
	"divide error",
	"debug",
	"non-maskable interrupt",
	"breakpoint",
	"overflow",
	"bound range exceeded",
	"invalid opcode",
	"device not available",
	"double fault",
	"coprocessor segment overrun",
	"invalid TSS",
	"segment not present",
	"stack-segment fault",
	"general protection fault",
	"page fault",
	"reserved",
	"x87 floating-point exception",
	"alignment check",
	"machine check",
	"SIMD floating-point exception",
	"virtualisation exception",
	"control protection exception",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"hypervisor injection exception",
	"VMM communication exception",
	"security exception",
	"reserved",
	"reserved",
};

const char *isr_exception_name(uint64_t vector)
{
	if (vector >= EXCEPTION_COUNT) {
		return "unknown";
	}
	return exception_names[vector];
}

void isr_register_handler(uint8_t vector, isr_handler_t handler)
{
	if (vector < VECTOR_COUNT) {
		handlers[vector] = handler;
	}
	return;
}

void isr_init(void)
{
	idt_init();
	pic_remap(PIC_REMAP_BASE, PIC_REMAP_BASE + 8);
	pic_disable();
	return;
}

void isr_enable(void)
{
	asm volatile ("sti");
	return;
}

static void panic(struct registers *regs)
{
	uint64_t cr2;
	asm volatile ("movq %%cr2, %0" : "=r" (cr2));

	kprintf("\n*** exception %llu: %s\n",
		(unsigned long long)regs->int_no,
		isr_exception_name(regs->int_no));
	kprintf("err=%#llx rip=%p cs=%#llx rflags=%#llx\n",
		(unsigned long long)regs->err_code, (void *)regs->rip,
		(unsigned long long)regs->cs,
		(unsigned long long)regs->rflags);
	kprintf("rsp=%p rax=%#llx rbx=%#llx rcx=%#llx rdx=%#llx\n",
		(void *)regs->rsp, (unsigned long long)regs->rax,
		(unsigned long long)regs->rbx, (unsigned long long)regs->rcx,
		(unsigned long long)regs->rdx);
	kprintf("rsi=%#llx rdi=%#llx rbp=%#llx cr2=%p\n",
		(unsigned long long)regs->rsi, (unsigned long long)regs->rdi,
		(unsigned long long)regs->rbp, (void *)cr2);
	kprintf("halted.\n");

	for (;;) {
		asm volatile ("cli; hlt");
	}
}

/* Called from isr_common in isr.S. */
void isr_handler(struct registers *regs);

void isr_handler(struct registers *regs)
{
	if (regs->int_no < EXCEPTION_COUNT) {
		if (handlers[regs->int_no] != NULL) {
			handlers[regs->int_no](regs);
			return;
		}
		panic(regs);
		return;
	}

	uint8_t irq = (uint8_t)(regs->int_no - PIC_REMAP_BASE);

	/* A spurious interrupt was never really raised, so the controller
	 * must not be acknowledged for it. */
	if (pic_is_spurious(irq)) {
		return;
	}

	if (regs->int_no < VECTOR_COUNT && handlers[regs->int_no] != NULL) {
		handlers[regs->int_no](regs);
	}

	pic_send_eoi(irq);
	return;
}
