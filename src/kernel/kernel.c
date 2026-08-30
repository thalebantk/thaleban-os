#include <limine.h>
#include <drivers/framebuffer.h>

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
	uint32_t COLOR[3];

	COLOR[0] = framebuffer_make_color(255, 0, 0);
	COLOR[1] = framebuffer_make_color(0, 255, 0);
	COLOR[2] = framebuffer_make_color(0, 0, 255);

	int color_count = 0;
	while (1)
	{
		for (uint64_t x = 0; x < fb_width(); x++)
		{
			for (uint64_t y = 0; y < fb_height(); y++)
			{
				framebuffer_put_pixel(x,  y, COLOR[color_count]);
			}
		}

		if (color_count < 3) color_count++;
		else color_count = 0;
	}

	hcf();
}
