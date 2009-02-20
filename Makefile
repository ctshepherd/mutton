CFLAGS = -O2 -fomit-frame-pointer -nostdinc -fno-builtin -Wall -W -Wextra -funsigned-char


all: kernel.bin

start.o: start.asm
	nasm -f aout -o start.o start.asm

string.o: string.c system.h
port.o: port.c
screen.o: screen.c system.h

kernel.bin: start.o link.ld main.o screen.o port.o string.o
	ld -T link.ld -o kernel.bin main.o start.o screen.o port.o string.o

clean:
	rm -f *.o kernel.bin
