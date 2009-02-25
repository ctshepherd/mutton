#ifndef _PORT_H
#define _PORT_H

static inline unsigned char inportb(unsigned short port)
{
	unsigned char rv;
	__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
	return rv;
}

static inline unsigned short inportw(unsigned short port)
{
	unsigned short rv;
	__asm__ __volatile__ ("inw %1, %0" : "=r" (rv) : "dN" (port));
	return rv;
}

static inline void outportb(unsigned short port, unsigned char data)
{
	__asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}

static inline void outportw(unsigned short port, unsigned short data)
{
	__asm__ __volatile__ ("outw %1, %0" : : "dN" (port), "r" (data));
}

#endif
