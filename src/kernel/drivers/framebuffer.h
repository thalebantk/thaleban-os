#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

void framebuffer_init(void);

uint64_t fb_width(void);

uint64_t fb_height(void);
uint32_t framebuffer_make_color(uint8_t r, uint8_t g, uint8_t b);
void framebuffer_put_pixel(uint64_t x, uint64_t y, uint32_t color);

#endif
