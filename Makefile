ASSEMBLER=nasm
CC=gcc
LD=ld

include src.mk

OBJECTS := $(ASM_SOURCES:.s=.o)
OBJECTS += $(C_SOURCES:.c=.o)

INCLUDES := -I./inc

CFLAGS := -Wall -Wextra  -nostdlib
CFLAGS += -fno-builtin -nostartfiles -nodefaultlibs

KERNEL := kernel.bin

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
	./update_image.sh && qemu -no-kvm -m 64  -fda floppy.img 
