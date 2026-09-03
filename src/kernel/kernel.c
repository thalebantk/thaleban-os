#include <limine.h>
#include <stddef.h>

#include <console/console.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/io.h>
#include <cpu/isr.h>
#include <drivers/framebuffer.h>
#include <drivers/pic.h>
#include <font/font.h>
#include <klib/kprintf.h>

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

static void hcf(void)
{
	for (;;) {
		asm volatile ("hlt");
	}
}

static void on_breakpoint(struct registers *regs)
{
	kprintf("  handler ran: vector %llu (%s), resume rip %p\n",
		(unsigned long long)regs->int_no,
		isr_exception_name(regs->int_no), (void *)regs->rip);
	return;
}

static void on_key(struct registers *regs)
{
	uint8_t scancode = inb(0x60);

	kprintf("  irq1 -> vector %llu  scancode %#04x (%s)\n",
		(unsigned long long)regs->int_no, scancode,
		(scancode & 0x80) ? "release" : "press");
	return;
}

void kernel_main(void)
{
	if (LIMINE_BASE_REVISION_SUPPORTED == 0) {
		hcf();
	}

	framebuffer_init();
	font_init();
	console_init();
	console_clear();

	uint32_t bg = framebuffer_make_color(16, 18, 24);
	uint32_t fg = framebuffer_make_color(220, 220, 220);
	uint32_t accent = framebuffer_make_color(120, 200, 255);

	console_set_color(accent, bg);
	kprintf("thaleban-os\n\n");
	console_set_color(fg, bg);

	gdt_init();
	isr_init();

	struct {
		uint16_t limit;
		uint64_t base;
	} __attribute__((packed)) live;
	asm volatile ("sidt %0" : "=m" (live));

	kprintf("idtr : base %p  limit %u (%u entries)\n",
		(void *)live.base, live.limit,
		(unsigned)(live.limit + 1) / 16);
	kprintf("pic  : master mask %#04x  slave mask %#04x\n",
		inb(0x21), inb(0xa1));

	isr_register_handler(3, on_breakpoint);
	isr_register_handler(PIC_REMAP_BASE + PIC_IRQ_KEYBOARD, on_key);

	kprintf("\nsoftware interrupt:\n");
	asm volatile ("int3");
	kprintf("  iretq returned to kernel_main\n");

	pic_unmask(PIC_IRQ_KEYBOARD);
	kprintf("\npic  : master mask %#04x after unmasking irq1\n", inb(0x21));

	isr_enable();
	kprintf("interrupts enabled, waiting for keys\n");

	hcf();
}
