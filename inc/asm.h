#ifndef __ASM_H_
#define __ASM_H_

#define ENABLE_INTERRUPT()    asm volatile ("sti");
#define CLEAR_INTERRUPT()     asm volatile ("cli");

#define yeild()               asm volatile("int $0x80");

#endif
