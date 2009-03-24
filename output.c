#include "stdarg.h"

static const char hex[] = "0123456789ABCDEF";

#define LENGTH 64
static char printf_buffer[LENGTH];

enum size {
	SDEFAULT,
	SCHAR,
	SHORT,
	INT,
	LONG,
	LONGLONG,
};

enum format {
	DEFAULT,
	CHAR,
	STRING,
	SIGNED,
	HEX,
};

static void parse_format(const char **_format, enum size *s, enum format *f,
		unsigned *base)
{
	char *format = *_format;

	switch (*format) {
	case 'l':
		format++;
		if (*format == 'l') {
			*s = LONGLONG;
			format++;
		} else {
			*s = LONG;
		}
		break;
	case 'h':
		format++;
		if (*format == 'h') {
			*s = SCHAR;
			format++;
		} else {
			*s = SHORT;
		}
		break;
	}

	switch (*format) {
	case 'c':
		*f = CHAR;
		break;
	case 's':
		*f = STRING;
		break;
	case 'p':
		*f = HEX;
		*s = LONG;
		*base = 16;
		break;
	case 'd':
	case 'i':
		*base = 10;
		*f = SIGNED;
		break;
	case 'u':
		*base = 10;
		break;
	case 'o':
		*base = 8;
		break;
	/* XXX: Handle these differently at some point */
	case 'x':
	case 'X':
		*f = HEX;
		*base = 16;
		break;
	default:
		;
		/* XXX: Assert: we should never get here */
	}

	*_format = ++format;
}

static unsigned decode_num(unsigned long output, unsigned base)
{
	unsigned i = 0;
	do {
		unsigned unit = (output % base);
		output /= base;
		printf_buffer[i++] = hex[unit];
	} while (output);
	return i;
}

#define OUTPUT_NUM(i) do {			\
	while (i--)				\
		output(printf_buffer[i]);	\
} while (0)

static unsigned output_num_formatted(unsigned long n, enum format f,
		unsigned base, void (*output)(char c))
{
	unsigned ret = 0;
	switch (f) {
	case CHAR:
		output((char)n);
		ret = 1;
		break;
	case SIGNED: {
		unsigned i;
		signed long s = n;
		if (s < 0)
			n = -s;
		i = decode_num(n, base);
		if (s < 0)
			printf_buffer[i++] = '-';
		ret = i;
		OUTPUT_NUM(i);
		break;
		}
	case HEX: {
		unsigned i;
		output('0');
		output('x');
		i = decode_num(n, base);
		ret = i+2;
		OUTPUT_NUM(i);
		break;
		}
	case DEFAULT: {
		unsigned i = decode_num(n, base);
		ret = i;
		OUTPUT_NUM(i);
		break;
		}
	}
	return ret;
}

int _vprintf(const char *format, va_list ap, void (*output)(char c))
{
	unsigned out = 0;
	while (*format) {
		unsigned base = 0;
		enum size s = SDEFAULT;
		enum format f = DEFAULT;
		if (*format != '%') {
			output(*format++);
			out++;
			continue;
		}
		format++;
		if (!*format)
			return -1;
		parse_format(&format, &s, &f, &base);
		if (f == STRING) {
			char *s = va_arg(ap, char *);
			char *o = s;
			while (*s)
				output(*s++);
			out += s - o;
			continue;
		}
		switch (s) {
		case SCHAR: {
			char c = va_arg(ap, int);
			out += output_num_formatted(c, f, base, output);
			continue;
			}
		case SHORT: {
			short s = va_arg(ap, int);
			out += output_num_formatted(s, f, base, output);
			continue;
			}
		case INT:
		case SDEFAULT: {
			unsigned i = va_arg(ap, int);
			out += output_num_formatted(i, f, base, output);
			continue;
			}
		case LONG: {
			long l = va_arg(ap, long);
			out += output_num_formatted(l, f, base, output);
			continue;
			}
		case LONGLONG: {
			/* XXX: This will cause an exception */
			char *s = 0x0;
			output(*s);
			}
		}
	}
	return out;
}
