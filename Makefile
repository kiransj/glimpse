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
	$(LD) -T asm/linker.ld -o $(KERNEL) $(OBJECTS)

%.o:%.s
	$(ASSEMBLER) -f elf -o $@  $^ 

%.o:%.c
	$(CC) $(INCLUDES) -o $@ -c $^ $(CFLAGS) 

clean:
	rm -f $(OBJECTS) $(KERNEL)
