#include <stddef.h>

#include <console/console.h>
#include <drivers/framebuffer.h>
#include <font/font.h>

#define TAB_WIDTH 8

static uint32_t cols;
static uint32_t rows;
static uint32_t col;
static uint32_t row;
static uint32_t fg;
static uint32_t bg;

void console_init(void)
{
	uint32_t w = font_width();
	uint32_t h = font_height();

	/* Zero metrics mean the font never parsed. cols/rows stay zero and
	 * every write below turns into a no-op. */
	cols = w == 0 ? 0 : (uint32_t)(fb_width() / w);
	rows = h == 0 ? 0 : (uint32_t)(fb_height() / h);
	col = 0;
	row = 0;

	fg = framebuffer_make_color(220, 220, 220);
	bg = framebuffer_make_color(16, 18, 24);
	return;
}

void console_clear(void)
{
	fb_clear(bg);
	col = 0;
	row = 0;
	return;
}

void console_set_color(uint32_t new_fg, uint32_t new_bg)
{
	fg = new_fg;
	bg = new_bg;
	return;
}

uint32_t console_cols(void) { return cols; }
uint32_t console_rows(void) { return rows; }

static void newline(void)
{
	col = 0;

	if (row + 1 < rows) {
		row++;
		return;
	}

	/* Parked on the last row: scroll instead of advancing. */
	fb_scroll_up(font_height(), bg);
	return;
}

void console_putchar(char c)
{
	if (cols == 0 || rows == 0) {
		return;
	}

	switch (c) {
	case '\n':
		newline();
		return;
	case '\r':
		col = 0;
		return;
	case '\b':
		if (col > 0) {
			col--;
			fb_draw_char(col * font_width(), row * font_height(),
				     ' ', fg, bg);
		}
		return;
	case '\t':
		do {
			console_putchar(' ');
		} while (col % TAB_WIDTH != 0);
		return;
	default:
		break;
	}

	if (col >= cols) {
		newline();
	}

	fb_draw_char(col * font_width(), row * font_height(), c, fg, bg);
	col++;
	return;
}

void console_write(const char *s)
{
	if (s == NULL) {
		return;
	}

	while (*s != '\0') {
		console_putchar(*s++);
	}
	return;
}
