#include <cpu/gdt.h>
#include <cpu/idt.h>

#define ISR_STUB_COUNT 48

struct idt_entry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t ist;
	uint8_t type_attr;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t reserved;
} __attribute__((packed));

struct idtr {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];
static struct idtr idtr;

/* Entry points from isr.S, one per vector the kernel handles. */
extern void *isr_stub_table[ISR_STUB_COUNT];

void idt_set_gate(uint8_t vector, void *handler, uint8_t type, uint8_t ist)
{
	uint64_t address = (uint64_t)handler;

	idt[vector].offset_low = (uint16_t)(address & 0xffff);
	idt[vector].selector = GDT_KERNEL_CODE;
	idt[vector].ist = ist & 0x07;
	idt[vector].type_attr = type;
	idt[vector].offset_mid = (uint16_t)((address >> 16) & 0xffff);
	idt[vector].offset_high = (uint32_t)(address >> 32);
	idt[vector].reserved = 0;
	return;
}

void idt_init(void)
{
	for (int i = 0; i < ISR_STUB_COUNT; i++) {
		idt_set_gate((uint8_t)i, isr_stub_table[i],
			     IDT_INTERRUPT_GATE, 0);
	}

	/* Vectors above the stub table stay absent: taking one raises #GP,
	 * which is handled, rather than jumping somewhere arbitrary. */

	idtr.limit = (uint16_t)(sizeof(idt) - 1);
	idtr.base = (uint64_t)&idt;

	asm volatile ("lidt %0" : : "m" (idtr));
	return;
}
