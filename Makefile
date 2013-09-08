ASSEMBLER=nasm
CC=gcc
LD=ld -m elf_i386 

include src.mk

OBJECTS := $(ASM_SOURCES:.s=.o)
OBJECTS += $(C_SOURCES:.c=.o)

INCLUDES := -I./inc

CFLAGS := -m32 -Wall -Wextra  -Werror -nostdlib #-DDEBUG
CFLAGS += -fno-builtin -nostartfiles -nodefaultlibs

KERNEL := output/kernel.bin

all: $(KERNEL)

$(KERNEL):$(OBJECTS) asm/linker.ld
	$(info building $(KERNEL))
	@$(LD) -T asm/linker.ld -o $(KERNEL) $(OBJECTS)

%.o:%.s
	$(info building $@)
	@$(ASSEMBLER) -f elf -o $@  $^

%.o:%.c
	$(info building $@)
	@$(CC) $(INCLUDES) -o $@ -c $^ $(CFLAGS)

clean:
	rm -f $(OBJECTS) $(KERNEL)

run: $(KERNEL)
	cd output && ./update_image.sh && qemu-system-i386 -no-kvm -m 64  -fda floppy.img 
