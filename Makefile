all: kernel.bin

start.o: start.asm
	nasm -f aout -o start.o start.asm

string.o: string.c system.h
port.o: port.c

kernel.bin: start.o link.ld main.o
	ld -T link.ld -o kernel.bin main.o start.o

clean:
	rm -f *.o kernel.bin
