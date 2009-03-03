#include "port.h"
#include "screen.h"
#include "system.h"

#define CAP_LOCK	0xA0
#define SCROLL_LOCK	0xA1
#define NUM_LOCK	0xA2

/* Scancode table used to layout a standard US keyboard. */
static char kbdus[128] = {
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	CAP_LOCK, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, /* Left shift */ '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,	/* Right shift */
	'*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Ctrl */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

#define BUFSIZE 127
static char kbuf[BUFSIZE+1];
static unsigned index = 0;

/* Handles the keyboard interrupt */
static void keyboard_handler(struct regs *r)
{
	unsigned char scancode;

	/* Read from the keyboard's data buffer */
	scancode = inportb(0x60);

	/* If the top bit of the byte we read from the keyboard is *  set,
	 * that means that a key has just been released */
	if (scancode & 0x80) {
		/* Has shift/alt/control been released? */
	} else {
		/* A key was just pressed. Keys held down will yield repeated
		 * key press interrupts. */

		/* Translate the keyboard scancode into an ASCII value, and
		 * add it to the buffer. */
		char c = kbdus[scancode];
		if (c)
			kbuf[index++] = c;
		if (index == BUFSIZE) {
			/* What do we do with the buffer? */
			kbuf[index] = '\0';
			index = 0;
		}
	}
}

void init_kbd(void)
{
	register_irq(1, keyboard_handler);
}
