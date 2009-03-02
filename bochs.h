#ifndef _BOCHS_H
#define _BOCHS_H

#include "stdarg.h"
#include "output.h"
#include "port.h"

/* Outputs a character to the debug console */
static inline void bochs_putch(char c)
{
	outportb(0xe9, c);
}

static inline void bochs_puts(const char *text)
{
	while (*text)
		bochs_putch(*text++);
}

/* Stops simulation and breaks into the debug console */
static inline void bochs_break(void)
{
	outportw(0x8A00, 0x8A00);
	outportw(0x8A00, 0x8AE0);
}

static inline int bochs_printf(const char *format, ...)
{
	va_list args;
	int ret;

	va_start(args, format);
	ret = _vprintf(format, args, bochs_putch);
	va_end(args);

	return ret;
}

#endif
