#ifndef IO_H
#define IO_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value)
{
	asm volatile ("outb %0, %1" : : "a" (value), "Nd" (port));
	return;
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t value;
	asm volatile ("inb %1, %0" : "=a" (value) : "Nd" (port));

	return value;
}

/* Port 0x80 is the POST diagnostic port, unused after boot. Writing to it
 * burns roughly a bus cycle, which older PICs need between command writes. */
static inline void io_wait(void)
{
	outb(0x80, 0);
	return;
}

#endif
