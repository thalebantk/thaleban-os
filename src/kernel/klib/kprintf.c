#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <console/console.h>
#include <klib/kprintf.h>

#define FLAG_LEFT  (1u << 0)
#define FLAG_ZERO  (1u << 1)
#define FLAG_PLUS  (1u << 2)
#define FLAG_SPACE (1u << 3)
#define FLAG_ALT   (1u << 4)

/* Base 2 of a 64-bit value is the widest conversion: 64 digits. */
#define DIGITS_MAX 64

enum length {
	LEN_INT,
	LEN_CHAR,
	LEN_SHORT,
	LEN_LONG,
	LEN_LONGLONG,
	LEN_SIZE,
	LEN_PTRDIFF,
};

static void emit(char c, int *count)
{
	console_putchar(c);
	(*count)++;
	return;
}

static void emit_pad(char c, int n, int *count)
{
	while (n-- > 0) {
		emit(c, count);
	}
	return;
}

/* Writes the digits of `v` into `buf` least significant first, and returns
 * how many there are. Callers walk the result backwards. */
static int to_digits(uint64_t v, uint32_t base, int upper, char *buf)
{
	static const char lower_set[] = "0123456789abcdef";
	static const char upper_set[] = "0123456789ABCDEF";
	const char *set = upper ? upper_set : lower_set;
	int n = 0;

	if (v == 0) {
		buf[n++] = '0';
		return n;
	}

	while (v != 0 && n < DIGITS_MAX) {
		buf[n++] = set[v % base];
		v /= base;
	}
	return n;
}

static int parse_int(const char **fmt)
{
	int n = 0;

	while (**fmt >= '0' && **fmt <= '9') {
		n = n * 10 + (**fmt - '0');
		(*fmt)++;
	}
	return n;
}

static void format_number(uint64_t value, uint32_t base, int upper, int negative,
			  unsigned flags, int width, int precision, int *count)
{
	char digits[DIGITS_MAX];
	int ndigits = to_digits(value, base, upper, digits);

	/* C says an explicit precision of 0 applied to a value of 0 yields no
	 * digits at all. */
	if (precision == 0 && value == 0) {
		ndigits = 0;
	}

	char sign = 0;
	if (negative) {
		sign = '-';
	} else if (flags & FLAG_PLUS) {
		sign = '+';
	} else if (flags & FLAG_SPACE) {
		sign = ' ';
	}

	char prefix[2] = { 0, 0 };
	if (flags & FLAG_ALT) {
		/* Only a nonzero result carries the 0x/0b prefix; octal is the
		 * exception, where '#' just guarantees a leading zero. */
		if (base == 16 && value != 0) {
			prefix[0] = '0';
			prefix[1] = upper ? 'X' : 'x';
		} else if (base == 2 && value != 0) {
			prefix[0] = '0';
			prefix[1] = 'b';
		} else if (base == 8 &&
			   (ndigits == 0 || digits[ndigits - 1] != '0')) {
			prefix[0] = '0';
		}
	}

	/* An explicit precision is a minimum digit count, and it overrides
	 * zero padding the way C's printf does. */
	int zeros = precision > ndigits ? precision - ndigits : 0;
	if (precision >= 0) {
		flags &= ~FLAG_ZERO;
	}

	int prefix_len = (prefix[0] != 0) + (prefix[1] != 0);
	int body = (sign != 0) + prefix_len + zeros + ndigits;
	int pad = width > body ? width - body : 0;

	if (pad > 0 && !(flags & (FLAG_LEFT | FLAG_ZERO))) {
		emit_pad(' ', pad, count);
		pad = 0;
	}

	if (sign != 0) {
		emit(sign, count);
	}
	for (int i = 0; i < prefix_len; i++) {
		emit(prefix[i], count);
	}

	if (pad > 0 && !(flags & FLAG_LEFT)) {
		/* FLAG_ZERO: padding goes after the sign and prefix. */
		emit_pad('0', pad, count);
		pad = 0;
	}

	emit_pad('0', zeros, count);
	while (ndigits-- > 0) {
		emit(digits[ndigits], count);
	}

	emit_pad(' ', pad, count);
	return;
}

static uint64_t fetch_unsigned(va_list *ap, enum length len)
{
	switch (len) {
	case LEN_CHAR:     return (unsigned char)va_arg(*ap, unsigned int);
	case LEN_SHORT:    return (unsigned short)va_arg(*ap, unsigned int);
	case LEN_LONG:     return va_arg(*ap, unsigned long);
	case LEN_LONGLONG: return va_arg(*ap, unsigned long long);
	case LEN_SIZE:     return va_arg(*ap, size_t);
	case LEN_PTRDIFF:  return (uint64_t)va_arg(*ap, long);
	default:           return va_arg(*ap, unsigned int);
	}
}

static int64_t fetch_signed(va_list *ap, enum length len)
{
	switch (len) {
	case LEN_CHAR:     return (signed char)va_arg(*ap, int);
	case LEN_SHORT:    return (short)va_arg(*ap, int);
	case LEN_LONG:     return va_arg(*ap, long);
	case LEN_LONGLONG: return va_arg(*ap, long long);
	case LEN_SIZE:     return va_arg(*ap, int64_t);
	case LEN_PTRDIFF:  return va_arg(*ap, long);
	default:           return va_arg(*ap, int);
	}
}

int kvprintf(const char *fmt, va_list ap_in)
{
	int count = 0;

	if (fmt == NULL) {
		return 0;
	}

	/* A va_list parameter has already decayed to a pointer, so &ap_in would
	 * be the wrong type to hand the fetch helpers. va_copy gives a real
	 * va_list object whose address they can take. */
	va_list ap;
	va_copy(ap, ap_in);

	while (*fmt != '\0') {
		if (*fmt != '%') {
			emit(*fmt++, &count);
			continue;
		}
		fmt++;

		unsigned flags = 0;
		for (;;) {
			if (*fmt == '-')      { flags |= FLAG_LEFT;  fmt++; }
			else if (*fmt == '0') { flags |= FLAG_ZERO;  fmt++; }
			else if (*fmt == '+') { flags |= FLAG_PLUS;  fmt++; }
			else if (*fmt == ' ') { flags |= FLAG_SPACE; fmt++; }
			else if (*fmt == '#') { flags |= FLAG_ALT;   fmt++; }
			else break;
		}

		int width = 0;
		if (*fmt == '*') {
			width = va_arg(ap, int);
			if (width < 0) {
				flags |= FLAG_LEFT;
				width = -width;
			}
			fmt++;
		} else {
			width = parse_int(&fmt);
		}

		int precision = -1;
		if (*fmt == '.') {
			fmt++;
			if (*fmt == '*') {
				precision = va_arg(ap, int);
				fmt++;
			} else {
				precision = parse_int(&fmt);
			}
			if (precision < 0) {
				precision = -1;
			}
		}

		enum length len = LEN_INT;
		if (*fmt == 'h') {
			fmt++;
			len = LEN_SHORT;
			if (*fmt == 'h') { fmt++; len = LEN_CHAR; }
		} else if (*fmt == 'l') {
			fmt++;
			len = LEN_LONG;
			if (*fmt == 'l') { fmt++; len = LEN_LONGLONG; }
		} else if (*fmt == 'z') {
			fmt++;
			len = LEN_SIZE;
		} else if (*fmt == 't') {
			fmt++;
			len = LEN_PTRDIFF;
		}

		char conv = *fmt;
		if (conv == '\0') {
			break;
		}
		fmt++;

		switch (conv) {
		case 'd':
		case 'i': {
			int64_t v = fetch_signed(&ap, len);
			/* Negated as unsigned so INT64_MIN survives. */
			uint64_t mag = v < 0 ? ~(uint64_t)v + 1 : (uint64_t)v;
			format_number(mag, 10, 0, v < 0, flags, width,
				      precision, &count);
			break;
		}
		case 'u':
			format_number(fetch_unsigned(&ap, len), 10, 0, 0, flags,
				      width, precision, &count);
			break;
		case 'o':
			format_number(fetch_unsigned(&ap, len), 8, 0, 0, flags,
				      width, precision, &count);
			break;
		case 'b':
			format_number(fetch_unsigned(&ap, len), 2, 0, 0, flags,
				      width, precision, &count);
			break;
		case 'x':
			format_number(fetch_unsigned(&ap, len), 16, 0, 0, flags,
				      width, precision, &count);
			break;
		case 'X':
			format_number(fetch_unsigned(&ap, len), 16, 1, 0, flags,
				      width, precision, &count);
			break;
		case 'p': {
			void *ptr = va_arg(ap, void *);
			format_number((uint64_t)(uintptr_t)ptr, 16, 0, 0,
				      flags | FLAG_ALT, width, precision,
				      &count);
			break;
		}
		case 'c': {
			char c = (char)va_arg(ap, int);
			int pad = width > 1 ? width - 1 : 0;

			if (!(flags & FLAG_LEFT)) {
				emit_pad(' ', pad, &count);
			}
			emit(c, &count);
			if (flags & FLAG_LEFT) {
				emit_pad(' ', pad, &count);
			}
			break;
		}
		case 's': {
			const char *s = va_arg(ap, const char *);
			if (s == NULL) {
				s = "(null)";
			}

			int len_s = 0;
			while (s[len_s] != '\0' &&
			       (precision < 0 || len_s < precision)) {
				len_s++;
			}

			int pad = width > len_s ? width - len_s : 0;
			if (!(flags & FLAG_LEFT)) {
				emit_pad(' ', pad, &count);
			}
			for (int i = 0; i < len_s; i++) {
				emit(s[i], &count);
			}
			if (flags & FLAG_LEFT) {
				emit_pad(' ', pad, &count);
			}
			break;
		}
		case '%':
			emit('%', &count);
			break;
		default:
			/* Unknown conversion: show it rather than swallow it. */
			emit('%', &count);
			emit(conv, &count);
			break;
		}
	}

	va_end(ap);
	return count;
}

int kprintf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int count = kvprintf(fmt, ap);
	va_end(ap);

	return count;
}
