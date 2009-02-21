CFLAGS = -O2 -fomit-frame-pointer -nostdinc -fno-builtin -Wall -W -Wextra -funsigned-char
OBJECTS = main.o string.o port.o screen.o gdt.o idt.o start.o

all: kernel.bin

start.o: start.asm
	nasm -f aout -o start.o start.asm

kernel.bin: link.ld $(OBJECTS)
	ld -T link.ld -o kernel.bin $(OBJECTS)

clean:
	rm -f *.o kernel.bin
