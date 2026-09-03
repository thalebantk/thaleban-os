#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

/* A character grid over the framebuffer: owns the cursor, wrapping and
 * scrolling, so callers emit a stream of characters and never track
 * coordinates. Requires framebuffer_init() and font_init() to have run. */
void console_init(void);

/* Clears the screen to the current background and homes the cursor. */
void console_clear(void);

void console_set_color(uint32_t fg, uint32_t bg);

uint32_t console_cols(void);
uint32_t console_rows(void);

/* Handles '\n', '\r', '\t' (to the next multiple of 8 columns) and '\b'.
 * Everything else occupies one cell, wrapping at the right edge and
 * scrolling at the bottom. */
void console_putchar(char c);

void console_write(const char *s);

#endif
