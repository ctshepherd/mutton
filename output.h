#include "stdarg.h"

int _vprintf(const char *format, va_list ap, void (*output)(char c));
