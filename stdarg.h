#ifndef _STDARG_H
#define _STDARG_H

/* Take verbatim from Pintos, thanks to Ben Pfaff:
 * GCC has <stdarg.h> functionality as built-ins, so all we need is to use it.
 */

typedef __builtin_va_list va_list;

#define va_start(LIST, ARG)	__builtin_va_start(LIST, ARG)
#define va_end(LIST)		__builtin_va_end(LIST)
#define va_arg(LIST, TYPE)	__builtin_va_arg(LIST, TYPE)
#define va_copy(DST, SRC)	__builtin_va_copy(DST, SRC)

#endif
