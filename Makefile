ifdef DEBUG
CFLAGS = -O0 -ggdb3
else
CFLAGS = -O2
endif

CFLAGS += -fno-builtin -Wall -W -Wextra -funsigned-char
OBJECTS =			\
	start.o			\
	gdt.o			\
	idt.o			\
	initrd.o		\
	isr_handlers.o		\
	isr.o			\
	kbd.o			\
	main.o			\
	malloc.o		\
	mboot.o			\
	output.o		\
	page.o			\
	pit.o			\
	screen.o		\
	string.o		\
	test_malloc.o		\
	vfs.o			\


all: kernel.bin

gdt.o: gdt.c system.h
idt.o: idt.c string.h system.h
isr.o: isr.c port.h screen.h system.h
main.o: main.c screen.h system.h
output.o: output.c output.h stdarg.h
screen.o: screen.c screen.h stdarg.h port.h string.h output.h
string.o: string.c string.h

install:
	sudo cp kernel.bin /boot/mutton.bin
	sudo cp initrd.img /boot/mutton_initrd.img

base_floppy.img: menu.lst prep_image.sh
	./prep_image.sh

initrd.img: mkinitrd
	./mkinitrd *.c

floppy.img: base_floppy.img initrd.img kernel.bin
	cp base_floppy.img floppy.img
	mkdir staging
	sudo mount -o loop floppy.img staging
	sudo cp kernel.bin staging/boot/grub/
	sudo cp initrd.img staging/boot/grub/
	sudo umount staging
	rmdir staging

disk: floppy.img

start.o: start.asm
	nasm -f elf -o start.o start.asm
isr_handlers.o: isr_handlers.asm
	nasm -f elf -o isr_handlers.o isr_handlers.asm

kernel.bin: link.ld $(OBJECTS)
	ld -T link.ld -o kernel.bin $(OBJECTS)

clean:
	rm -f *.o kernel.bin initrd.img mkinitrd
