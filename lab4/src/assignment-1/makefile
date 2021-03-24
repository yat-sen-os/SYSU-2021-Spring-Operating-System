ASM_COMPILER = nasm
C_COMPLIER = gcc
C_COMPLIER_FLAGS = -O0 -Wall -march=i386 -m32 -nostdlib -fno-builtin -ffreestanding -fno-pic
LINKER = ld

hd.img : mbr.bin bootloader.bin
	dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
	dd if=bootloader.bin of=hd.img bs=512 count=5 seek=1 conv=notrunc

mbr.bin : mbr.asm
	$(ASM_COMPILER) -o mbr.bin -f bin mbr.asm

bootloader.bin : bootloader.o asm_utils.o setup.o
	$(LINKER) -o bootloader.bin -melf_i386 -N bootloader.o asm_utils.o setup.o -Ttext 0x0 --oformat binary

asm_utils.o : asm_utils.asm
	$(ASM_COMPILER) -o asm_utils.o -f elf32 asm_utils.asm

bootloader.o : bootloader.asm
	$(ASM_COMPILER) -o bootloader.o -f elf32 bootloader.asm

setup.o : setup.c
	$(C_COMPLIER) -o setup.o $(C_COMPLIER_FLAGS) -c setup.c

clean:
	rm *.o
