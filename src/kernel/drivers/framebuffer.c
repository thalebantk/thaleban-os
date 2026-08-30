#include <limine.h>
#include <stddef.h>

#include <drivers/framebuffer.h>

static struct limine_framebuffer *fb = NULL;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0,
};


void framebuffer_init(void)
{
	if (framebuffer_request.response == NULL ||
	    framebuffer_request.response->framebuffer_count < 1) {
		for (;;) {
			asm volatile ("hlt");
		}
	}
	fb = framebuffer_request.response->framebuffers[0];
	return;
}

uint64_t fb_width(void) { return fb->width; }
uint64_t fb_height(void) { return fb->height; }

uint32_t framebuffer_make_color(uint8_t r, uint8_t g, uint8_t b)
{
	return ((uint32_t)r << fb->red_mask_shift)
		| ((uint32_t)g << fb->green_mask_shift)
		| ((uint32_t)b << fb->blue_mask_shift);
}

void framebuffer_put_pixel(uint64_t x, uint64_t y, uint32_t color)
{
	if (fb == NULL || x >= fb->width || y >= fb->height) {
		return;
	}

	volatile uint32_t *pixels = (volatile uint32_t *)fb->address;
	pixels[y * (fb->pitch / 4) + x] = color;
}
