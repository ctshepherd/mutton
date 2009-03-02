CFLAGS = -O2 -fomit-frame-pointer -nostdinc -fno-builtin -Wall -W -Wextra -funsigned-char
OBJECTS = main.o string.o screen.o gdt.o idt.o start.o helper.o isr.o isr_handlers.o output.o

all: kernel.bin

gdt.o: gdt.c system.h
idt.o: idt.c string.h system.h
isr.o: isr.c screen.h system.h
main.o: main.c screen.h system.h
output.o: output.c output.h stdarg.h
screen.o: screen.c screen.h stdarg.h port.h screen.h string.h output.h
string.o: string.c string.h

install:
	sudo cp kernel.bin /boot/mutton.bin

pad:
	dd if=/dev/zero of=pad count=1 bs=750

floppy.img: stage1 stage2 pad kernel.bin
	cat stage1 stage2 pad kernel.bin >floppy.img
	python -c 'from math import ceil; print int(ceil(len(open("kernel.bin").read())/512))'

disk: floppy.img

start.o: start.asm
	nasm -f elf -o start.o start.asm
helper.o: helper.asm
	nasm -f elf -o helper.o helper.asm
isr_handlers.o: isr_handlers.asm
	nasm -f elf -o isr_handlers.o isr_handlers.asm

kernel.bin: link.ld $(OBJECTS)
	ld -T link.ld -o kernel.bin $(OBJECTS)

clean:
	rm -f *.o kernel.bin
