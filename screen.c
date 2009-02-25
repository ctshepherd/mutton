#include "port.h"
#include "system.h"

/* More information at http://www.brackeen.com/home/vga */


/*
 * These define our textpointer, our background and foreground colors
 * (attributes), and x and y cursor coordinates.
 */
static unsigned short *textmemptr = (unsigned short *)0xB8000;
static int attrib = 0x0F;
static int csr_x = 0, csr_y = 0;

/* Scrolls the screen */
void scroll(void)
{
	unsigned blank, temp;

	/* A blank is defined as a space - we need to give it backcolor too */
	blank = 0x20 | (attrib << 8);

	/* Row 25 is the end, this means we need to scroll up */
	if (csr_y >= 25) {
		/*
		 * Move the current text chunk that makes up the screen back
		 * in the buffer by a line
		 */
		temp = csr_y - 25 + 1;
		memcpy(textmemptr, textmemptr + temp * 80, (25 - temp) * 80 * 2);

		/*
		 * Finally, we set the chunk of memory that occupies the last
		 * line of text to our 'blank' character
		 */
		memset(textmemptr + (25 - temp) * 80, blank, 80);
		csr_y = 25 - 1;
	}
}

/* Updates the hardware cursor */
void move_csr(void)
{
	unsigned temp;

	/*
	 * The equation for finding the index in a linear chunk of memory can
	 * be represented by:
	 *   Index = [(y * width) + x]
	 */
	temp = csr_y * 80 + csr_x;

	/*
	 * This sends a command to indicies 14 and 15 in the CRT Control
	 * Register of the VGA controller. These are the high and low bytes of
	 * the index that show where the hardware cursor is to be 'blinking'.
	 */
	outportb(0x3D4, 14);
	outportb(0x3D5, temp >> 8);
	outportb(0x3D4, 15);
	outportb(0x3D5, temp);
}

/* Clears the screen */
void cls(void)
{
	unsigned blank;
	int i;

	/* Again, we need the 'short' that will be used to
	*  represent a space with color */
	blank = 0x20 | (attrib << 8);

	/* Sets the entire screen to spaces in our current
	*  color */
	for (i = 0; i < 25; i++)
		memset(textmemptr + i * 80, blank, 80);

	/* Update out virtual cursor, and then move the
	*  hardware cursor */
	csr_x = 0;
	csr_y = 0;
	move_csr();
}

/* Puts a single character on the screen */
void putch(char c)
{
	unsigned short *where;
	unsigned att = attrib << 8;

	if (c == 0x08) {
		/* Handle a backspace, by moving the cursor back one space */
		if (csr_x != 0)
			csr_x--;
	} else if (c == 0x09) {
		/* Handles a tab by incrementing the cursor's x, but only to a
		 * point that will make it divisible by 8 */
		csr_x = (csr_x + 8) & ~(8 - 1);
	} else if (c == '\r') {
		/* Handles a 'Carriage Return', which simply brings the cursor
		 * back to the margin */
		csr_x = 0;
	} else if (c == '\n') {
		/* We handle our newlines the way DOS and the BIOS do: we
		 * treat it as if a 'CR' was also there, so we bring the
		 * cursor to the margin and we increment the 'y' value */
		csr_x = 0;
		csr_y++;
	} else if (c >= ' ') {
		/* Any character greater than and including a space, is a
		 * printable character. The equation for finding the index in
		 * a linear chunk of memory can be represented by:
		 *   Index = [(y width) + x] */
		where = textmemptr + (csr_y * 80 + csr_x);
		*where = c | att;	/* Character AND attributes: color */
		csr_x++;
	}

	/* If the cursor has reached the edge of the screen's width, we insert
	 * a new line in there */
	if (csr_x >= 80) {
		csr_x = 0;
		csr_y++;
	}

	/* Scroll the screen if needed, and finally move the cursor */
	scroll();
	move_csr();
}

/* Uses the above routine to output a string... */
void puts(const char *text)
{
	while (*text)
		putch(*text++);
}

/* Sets the forecolor and backcolor that we will use */
void settextcolor(unsigned char forecolor, unsigned char backcolor)
{
	/* Top 4 bytes are the background, bottom 4 bytes are the foreground
	 * color */
	attrib = (backcolor << 4) | (forecolor & 0x0F);
}

/* Sets our text-mode VGA pointer, then clears the screen for us */
void init_video(void)
{
	cls();
}
