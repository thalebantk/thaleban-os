#include <limine.h>
#include <stddef.h>

#include <drivers/framebuffer.h>
#include <font/font.h>

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

void fb_clear(uint32_t color)
{
	if (fb == NULL) {
		return;
	}

	volatile uint32_t *pixels = (volatile uint32_t *)fb->address;
	uint64_t stride = fb->pitch / 4;

	for (uint64_t y = 0; y < fb->height; y++) {
		for (uint64_t x = 0; x < fb->width; x++) {
			pixels[y * stride + x] = color;
		}
	}
	return;
}

void fb_draw_char(uint64_t x, uint64_t y, char c, uint32_t fg, uint32_t bg)
{
	const uint8_t *glyph = font_glyph(c);
	if (glyph == NULL) {
		return;
	}

	uint32_t width = font_width();
	uint32_t height = font_height();
	uint32_t stride = font_stride();

	for (uint32_t row = 0; row < height; row++) {
		const uint8_t *bits = glyph + (uint64_t)row * stride;

		for (uint32_t col = 0; col < width; col++) {
			uint8_t set = bits[col / 8] & (0x80u >> (col % 8));
			framebuffer_put_pixel(x + col, y + row, set ? fg : bg);
		}
	}
	return;
}

void fb_puts(uint64_t x, uint64_t y, const char *s, uint32_t fg, uint32_t bg)
{
	uint32_t width = font_width();
	uint32_t height = font_height();

	if (s == NULL || width == 0 || height == 0) {
		return;
	}

	uint64_t origin = x;

	while (*s != '\0') {
		char c = *s++;

		if (c == '\n') {
			x = origin;
			y += height;
			continue;
		}
		if (c == '\r') {
			x = origin;
			continue;
		}

		if (x + width > fb_width()) {
			x = origin;
			y += height;
		}
		if (y + height > fb_height()) {
			return;
		}

		fb_draw_char(x, y, c, fg, bg);
		x += width;
	}
	return;
}

void fb_scroll_up(uint64_t pixels, uint32_t fill)
{
	if (fb == NULL || pixels == 0) {
		return;
	}
	if (pixels >= fb->height) {
		fb_clear(fill);
		return;
	}

	volatile uint32_t *px = (volatile uint32_t *)fb->address;
	uint64_t stride = fb->pitch / 4;

	for (uint64_t y = 0; y + pixels < fb->height; y++) {
		for (uint64_t x = 0; x < fb->width; x++) {
			px[y * stride + x] = px[(y + pixels) * stride + x];
		}
	}

	for (uint64_t y = fb->height - pixels; y < fb->height; y++) {
		for (uint64_t x = 0; x < fb->width; x++) {
			px[y * stride + x] = fill;
		}
	}
	return;
}
