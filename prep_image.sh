# Taken from Pedigree

mkdir fd
chmod 0777 fd

dd if=/dev/zero of=base_floppy.img bs=1024 count=1440
sudo mke2fs -F base_floppy.img

sudo mount -o loop base_floppy.img fd
sudo mkdir -p fd/boot/grub
sudo cp stage1 fd/boot/grub
sudo cp stage2 fd/boot/grub
sudo cp menu.lst fd/boot/grub
sudo umount fd

sudo grub --batch --no-floppy <<EOT 1>/dev/null  || exit 1
device (fd0) base_floppy.img
install (fd0)/boot/grub/stage1 (fd0) (fd0)/boot/grub/stage2 (fd0)/boot/grub/menu.lst
quit
EOT

rmdir fd
