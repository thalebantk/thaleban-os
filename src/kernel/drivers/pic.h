#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/* The 8259s power up mapping IRQ 0-15 onto vectors 8-15 and 0x70-0x77. The
 * first range collides with the CPU's own exceptions -- a timer tick would
 * arrive as #DF -- so the controllers must be remapped before interrupts are
 * enabled. 0x20 is the first vector Intel leaves free. */
#define PIC_REMAP_BASE 0x20

#define PIC_IRQ_TIMER    0
#define PIC_IRQ_KEYBOARD 1
#define PIC_IRQ_CASCADE  2

/* Remaps both controllers so IRQ 0-7 land on `offset` and IRQ 8-15 on
 * `offset + 8`, leaving every line masked. */
void pic_remap(uint8_t offset, uint8_t offset2);

void pic_mask(uint8_t irq);
void pic_unmask(uint8_t irq);

/* Masks every line. Use before switching to the APIC. */
void pic_disable(void);

void pic_send_eoi(uint8_t irq);

/* True when the controller reports no line actually in service, which means
 * the interrupt was electrical noise and must not be acknowledged. */
int pic_is_spurious(uint8_t irq);

#endif
