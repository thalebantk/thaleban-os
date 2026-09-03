#ifndef FONT_H
#define FONT_H

#include <stdint.h>

/* Parses the embedded PSF2 font. Until this succeeds font_glyph() returns
 * NULL and the metrics below read zero, so a malformed font costs a blank
 * screen rather than a fault. */
void font_init(void);

uint32_t font_width(void);
uint32_t font_height(void);

/* Bytes per glyph row. Rows are padded to whole bytes, so the 14-pixel wide
 * Terminus glyphs carry two bytes with the low two bits unused. */
uint32_t font_stride(void);

/* Bitmap for one character, font_height() rows of font_stride() bytes, MSB
 * first: the leftmost pixel of a row is bit 7 of its first byte. Characters
 * the font does not cover fall back to '?'; NULL means no usable font. */
const uint8_t *font_glyph(char c);

#endif
