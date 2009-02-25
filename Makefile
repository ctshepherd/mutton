CFLAGS = -O2 -fomit-frame-pointer -nostdinc -fno-builtin -Wall -W -Wextra -funsigned-char
OBJECTS = main.o string.o port.o screen.o gdt.o idt.o start.o helper.o isr.o

all: kernel.bin

install:
	sudo cp kernel.bin /boot/mutton.bin

start.o: start.asm
	nasm -f elf -o start.o start.asm
helper.o: helper.asm
	nasm -f elf -o helper.o helper.asm

kernel.bin: link.ld $(OBJECTS)
	ld -T link.ld -o kernel.bin $(OBJECTS)

clean:
	rm -f *.o kernel.bin
