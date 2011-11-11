ASM_SOURCES += asm/loader.s
ASM_SOURCES += asm/gdt.s
ASM_SOURCES += asm/interrupt.s

C_SOURCES  = src/kernel.c
C_SOURCES += src/util.c
C_SOURCES += src/display.c
C_SOURCES += src/descriptor_tables.c
C_SOURCES += src/isr.c
C_SOURCES += src/timer.c
C_SOURCES += src/paging.c
C_SOURCES += src/ram_manager.c
C_SOURCES += src/memory.c
C_SOURCES += src/schedule.c


