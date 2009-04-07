#ifndef _TYPE_H
#define _TYPE_H

#define __noreturn__		__attribute__((noreturn))
#define __printf__(a, b)	__attribute__((format(printf, a, b)))

#undef NULL
#define NULL			((void *) 0)

typedef unsigned		size_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long long	uint64_t;

#endif
