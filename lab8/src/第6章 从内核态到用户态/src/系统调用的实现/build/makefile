# 首先执行 make
# 然后执行 make build

ASM_COMPILER = nasm
C_COMPLIER = gcc
CXX_COMPLIER = g++
CXX_COMPLIER_FLAGS = -O0 -Wall -march=i386 -m32 -nostdlib -fno-builtin -ffreestanding -fno-pic
LINKER = ld

SRCDIR = ../src
RUNDIR = ../run
BUILDDIR = build
INCLUDE_PATH = ../include

CXX_SOURCE += $(wildcard $(SRCDIR)/kernel/*.cpp)
CXX_SOURCE += $(wildcard $(SRCDIR)/interrupt/*.cpp)
CXX_SOURCE += $(wildcard $(SRCDIR)/io/*.cpp)
CXX_SOURCE += $(wildcard $(SRCDIR)/utils/*.cpp)
CXX_SOURCE += $(wildcard $(SRCDIR)/memory/*.cpp)
CXX_SOURCE += $(wildcard $(SRCDIR)/program/*.cpp)
OBJ = $(wildcard *.o)

complie : $(OBJ)
	@$(ASM_COMPILER) -o entry.obj -f elf32 $(SRCDIR)/boot/entry.asm
	@$(CXX_COMPLIER) $(CXX_COMPLIER_FLAGS) -I$(INCLUDE_PATH) -c $(CXX_SOURCE)
	@$(ASM_COMPILER) -o asm_utils.o -f elf32 -I$(INCLUDE_PATH)/ $(SRCDIR)/utils/asm_utils.asm

build : mbr.bin bootloader.bin kernel.bin
	dd if=mbr.bin of=$(RUNDIR)/hd.img bs=512 count=1 seek=0 conv=notrunc
	dd if=bootloader.bin of=$(RUNDIR)/hd.img bs=512 count=5 seek=1 conv=notrunc
	dd if=kernel.bin of=$(RUNDIR)/hd.img bs=512 count=145 seek=6 conv=notrunc
	rm -f $(RUNDIR)/*.lock
# nasm的include path有一个尾随/
mbr.bin : $(SRCDIR)/boot/mbr.asm
	@$(ASM_COMPILER) -o mbr.bin -f bin -I$(INCLUDE_PATH)/ $(SRCDIR)/boot/mbr.asm

bootloader.bin : $(SRCDIR)/boot/bootloader.asm 
	@$(ASM_COMPILER) -o bootloader.bin -f bin -I$(INCLUDE_PATH)/ $(SRCDIR)/boot/bootloader.asm

kernel.bin : $(OBJ)
	@$(LINKER) -o kernel.bin -melf_i386 -N entry.obj $(OBJ) -Ttext 0x00020000 --oformat binary

clean:
	@rm -f *.o* *.bin 

test:
	@echo $(OBJ) 
