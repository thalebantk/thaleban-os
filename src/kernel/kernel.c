#include <limine.h>
#include <drivers/framebuffer.h>
#include <font/font.h>

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

	uint32_t bg = framebuffer_make_color(16, 18, 24);
	uint32_t fg = framebuffer_make_color(220, 220, 220);
	uint32_t accent = framebuffer_make_color(120, 200, 255);

	fb_clear(bg);

	fb_puts(font_width(), font_height(), "thaleban-os", accent, bg);
	fb_puts(font_width(), font_height() * 3,
		"Terminus 14x28, PSF2, linked into the kernel image.\n"
		"abcdefghijklmnopqrstuvwxyz\n"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
		"0123456789 !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",
		fg, bg);

	hcf();
}
