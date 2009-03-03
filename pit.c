#include "port.h"
#include "system.h"

static unsigned long seconds = 0;

#define HERTZ 100

static unsigned cur_hz = 18;
void timer_phase(int hz)
{
	int divisor = 1193180 / hz;       /* Calculate our divisor */
	cur_hz = hz;
	outportb(0x43, 0x36);             /* Set our command byte 0x36 */
	outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
	outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

void pit_timer(struct regs *r)
{
	static unsigned long count = 0;
	count++;
	if (count % cur_hz)
		seconds++;
}

unsigned long gettimeofday(void)
{
	return seconds;
}

void init_pit(void)
{
	register_irq(0, pit_timer);
	timer_phase(HERTZ);
}
