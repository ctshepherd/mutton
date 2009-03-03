struct regs;

void init_pit(void);
void timer_phase(int hz);
void pit_timer(struct regs *r);
unsigned long gettimeofday(void);
