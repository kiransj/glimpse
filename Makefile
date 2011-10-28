ASSEMBLER=nasm
CC=gcc
LD=ld

include src.mk

OBJECTS := $(ASM_SOURCES:.s=.o)
OBJECTS += $(C_SOURCES:.c=.o)

INCLUDES := -I./inc

CFLAGS := -Wall -Wextra -Werror -nostdlib 
CFLAGS += -fno-builtin -nostartfiles -nodefaultlibs

KERNEL := kernel.bin

all: $(KERNEL)

$(KERNEL):$(OBJECTS)
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
run:	
	qemu -no-kvm -kernel $(KERNEL)
