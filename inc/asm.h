#ifndef __ASM_H_
#define __ASM_H_

#define ENABLE_INTERRUPT()    asm volatile ("sti");
#define CLEAR_INTERRUPT()     asm volatile ("cli");

#define GET_ESP(x)  asm volatile("mov %%esp, %0": "=r"(x))


#endif
