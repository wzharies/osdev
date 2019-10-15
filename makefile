C_SOURCES = $(shell find ./kernel -name "*.c")
C_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES))
S_SOURCES = $(shell find ./kernel -name "*.s")
S_OBJECTS = $(patsubst %.s, %.o, $(S_SOURCES))

CC = i386-elf-gcc
LD = i386-elf-ld
#CC = gcc
#LD = ld
ASM = nasm
ASIB = -I ./bootloader/ 
ENTRY_POINT = 0xc0001500

C_FLAGS = -c -Wall -m32 -ggdb -gstabs+ -nostdinc -fno-pic -fno-builtin -fno-stack-protector -I kernel/include/
LD_FLAGS = -m elf_i386  -Ttext  $(ENTRY_POINT) -e main -nostdlib
ASM_FLAGS = -f elf -g -F stabs


all: $(S_OBJECTS) $(C_OBJECTS) mbr.bin load.bin link update_image

#$(C_OBJECTS):$(C_SOURCES)
.c.o:
	@echo 编译内核代码文件 $< ...
	$(CC) $(C_FLAGS) $< -o $@

$(S_OBJECTS):$(S_SOURCES)
	@echo 编译内核汇编文件 $< ...
	$(ASM) $(ASM_FLAGS) $<

mbr.bin:./bootloader/mbr.S
	@echo 编译启动汇编文件 $< ...
	$(ASM) $(ASIB)  $< -o $@

load.bin:./bootloader/load.S
	@echo 编译启动汇编文件 $< ...
	$(ASM) $(ASIB)  $< -o $@

link:
	@echo 链接内核文件...
	$(LD) $(LD_FLAGS) $(S_OBJECTS) $(C_OBJECTS) -o kernel.bin

.PHONY:clean
clean:
	$(RM) $(S_OBJECTS) $(C_OBJECTS) kernel.bin mbr.bin load.bin

.PHONY: update_image
update_image:
	dd if=mbr.bin of=floppy.img bs=512 count=1 conv=notrunc
	dd if=load.bin of=floppy.img bs=512 count=4 seek=2 conv=notrunc
	dd if=kernel.bin of=floppy.img bs=512 count=200 seek=9 conv=notrunc


.PHONY:qemu 
qemu:
	qemu-system-i386 -m 512 -drive format=raw,file=floppy.img,index=0,media=disk -boot c

.PHONY:debug 
debug:
	qemu-system-i386 -m 512 -drive format=raw,file=floppy.img,index=0,media=disk -S -s -boot c &
	sleep 1
	gdb  -x gdbinit

.PHONY:bochs
bochs:
	bochs -f bochsrc.disk.book