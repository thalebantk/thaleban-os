#include <limine.h>
#include <console/console.h>
#include <cpu/gdt.h>
#include <klib/kprintf.h>
#include <drivers/framebuffer.h>
#include <font/font.h>
#include <stddef.h>

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

void kernel_main(void)
{
	if (LIMINE_BASE_REVISION_SUPPORTED == 0) {
		hcf();
	}

	framebuffer_init();
	font_init();
	console_init();
	console_clear();

	uint32_t accent = framebuffer_make_color(120, 200, 255);
	uint32_t fg = framebuffer_make_color(220, 220, 220);
	uint32_t bg = framebuffer_make_color(16, 18, 24);

	console_set_color(accent, bg);
	kprintf("thaleban-os\n\n");
	console_set_color(fg, bg);

	gdt_init();

	/* Read the state back out of the CPU: sgdt reports the table the
	 * hardware is actually using, and str the loaded task register. */
	struct {
		uint16_t limit;
		uint64_t base;
	} __attribute__((packed)) live;
	uint16_t cs, ss, tr;

	asm volatile ("sgdt %0" : "=m" (live));
	asm volatile ("movw %%cs, %0" : "=r" (cs));
	asm volatile ("movw %%ss, %0" : "=r" (ss));
	asm volatile ("str %0" : "=r" (tr));

	unsigned count = (unsigned)(live.limit + 1) / 8;

	kprintf("gdtr     : base %p  limit %u (%u entries)\n",
		(void *)live.base, live.limit, count);
	kprintf("selectors: cs=%#04x ss=%#04x tr=%#04x\n", cs, ss, tr);
	kprintf("\n");

	static const char *const names[] = {
		"null", "kernel code", "kernel data",
		"user data", "user code", "tss (low)", "tss (high)",
	};
	const uint64_t *desc = (const uint64_t *)live.base;

	for (unsigned i = 0; i < count; i++) {
		kprintf("  [%u] %#018llx  %s\n", i,
			(unsigned long long)desc[i], names[i]);
	}

	hcf();
}
