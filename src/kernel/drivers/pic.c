#include <cpu/io.h>
#include <drivers/pic.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xa0
#define PIC2_DATA    0xa1

#define ICW1_INIT 0x11 /* begin initialisation, ICW4 to follow */
#define ICW4_8086 0x01 /* 8086 mode rather than MCS-80/85 */

#define PIC_EOI      0x20
#define PIC_READ_ISR 0x0b

static uint8_t pic_offset = PIC_REMAP_BASE;

void pic_remap(uint8_t offset, uint8_t offset2)
{
	uint8_t mask1 = inb(PIC1_DATA);
	uint8_t mask2 = inb(PIC2_DATA);

	pic_offset = offset;

	outb(PIC1_COMMAND, ICW1_INIT);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT);
	io_wait();

	/* ICW2: the vector each controller's IRQ 0 maps to. */
	outb(PIC1_DATA, offset);
	io_wait();
	outb(PIC2_DATA, offset2);
	io_wait();

	/* ICW3: how the two are wired together. The master names the line the
	 * slave hangs off as a bitmask, the slave names it as a number. */
	outb(PIC1_DATA, 1 << PIC_IRQ_CASCADE);
	io_wait();
	outb(PIC2_DATA, PIC_IRQ_CASCADE);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	/* Initialisation clears the masks, so put back what was there and let
	 * callers open the lines they actually handle. */
	outb(PIC1_DATA, mask1);
	outb(PIC2_DATA, mask2);
	return;
}

void pic_mask(uint8_t irq)
{
	uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
	uint8_t bit = irq < 8 ? irq : (uint8_t)(irq - 8);

	outb(port, (uint8_t)(inb(port) | (1u << bit)));
	return;
}

void pic_unmask(uint8_t irq)
{
	uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
	uint8_t bit = irq < 8 ? irq : (uint8_t)(irq - 8);

	outb(port, (uint8_t)(inb(port) & ~(1u << bit)));

	/* An IRQ on the slave only reaches the CPU if the cascade line on the
	 * master is open too. */
	if (irq >= 8) {
		outb(PIC1_DATA,
		     (uint8_t)(inb(PIC1_DATA) & ~(1u << PIC_IRQ_CASCADE)));
	}
	return;
}

void pic_disable(void)
{
	outb(PIC1_DATA, 0xff);
	outb(PIC2_DATA, 0xff);
	return;
}

void pic_send_eoi(uint8_t irq)
{
	/* A slave interrupt passes through the master, so both need telling. */
	if (irq >= 8) {
		outb(PIC2_COMMAND, PIC_EOI);
	}
	outb(PIC1_COMMAND, PIC_EOI);
	return;
}

int pic_is_spurious(uint8_t irq)
{
	/* Only the lowest line of each controller can go spurious. */
	if (irq != 7 && irq != 15) {
		return 0;
	}

	uint16_t command = irq == 7 ? PIC1_COMMAND : PIC2_COMMAND;
	outb(command, PIC_READ_ISR);

	uint8_t in_service = inb(command);
	if (in_service & (1u << (irq & 7))) {
		return 0;
	}

	/* Spurious on the slave still reached the CPU through the master, so
	 * the master alone must be acknowledged. */
	if (irq == 15) {
		outb(PIC1_COMMAND, PIC_EOI);
	}
	return 1;
}
