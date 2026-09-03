#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>

/* Formatted output to the console. Returns the number of characters emitted.
 *
 * Conversions: d i u x X o b c s p %
 * Flags:       '-' left align, '0' zero pad, '+' and ' ' sign, '#' alt form
 * Width and precision:  numeric or '*'
 * Length:      hh h l ll z t
 *
 * There is deliberately no floating point support: the kernel is built with
 * -mno-80387 and -mno-sse, so the FPU is never touched.
 */
__attribute__((format(printf, 1, 2)))
int kprintf(const char *fmt, ...);

int kvprintf(const char *fmt, va_list ap);

#endif
