#ifndef _TYPE_H
#define _TYPE_H

#define __noreturn__		__attribute__((noreturn))
#define __printf__(a, b)	__attribute__((format(printf, a, b)))

typedef unsigned size_t;

#endif
