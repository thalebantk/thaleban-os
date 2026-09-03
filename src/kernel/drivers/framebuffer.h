#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

void framebuffer_init(void);

uint64_t fb_width(void);

uint64_t fb_height(void);
uint32_t framebuffer_make_color(uint8_t r, uint8_t g, uint8_t b);
void framebuffer_put_pixel(uint64_t x, uint64_t y, uint32_t color);
void fb_clear(uint32_t color);

/* Text rendering. Both draw opaquely: set pixels get fg, clear pixels get bg.
 * (x, y) is the top-left corner of the character cell. Glyphs come from the
 * font module, so font_init() must have run first. */
void fb_draw_char(uint64_t x, uint64_t y, char c, uint32_t fg, uint32_t bg);

/* Advances one cell per character. '\n' returns to the starting column and
 * drops a row, '\r' returns to the starting column, and text that runs past
 * the right edge wraps. */
void fb_puts(uint64_t x, uint64_t y, const char *s, uint32_t fg, uint32_t bg);

#endif
