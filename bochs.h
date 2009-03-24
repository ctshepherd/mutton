#ifndef _BOCHS_H
#define _BOCHS_H

#include "output.h"
#include "port.h"
#include "stdarg.h"
#include "type.h"

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

static inline int __printf__(1, 2) bochs_printf(const char *format, ...)
{
	va_list args;
	int ret;

	va_start(args, format);
	ret = _vprintf(format, args, bochs_putch);
	va_end(args);

	return ret;
}

#endif
