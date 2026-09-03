#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256

/* Gate types. An interrupt gate clears IF on entry so the handler cannot be
 * re-entered by another interrupt; a trap gate leaves it set. */
#define IDT_INTERRUPT_GATE 0x8e /* present, DPL 0, type 0xE */
#define IDT_TRAP_GATE      0x8f /* present, DPL 0, type 0xF */

/* Builds the table from the assembly stubs and loads it with lidt. */
void idt_init(void);

/* `ist` selects a stack from the TSS (1-7), or 0 to stay on the current one.
 * Needed for faults that cannot trust RSP, such as #DF. */
void idt_set_gate(uint8_t vector, void *handler, uint8_t type, uint8_t ist);

#endif
