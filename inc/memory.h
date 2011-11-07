#ifndef __MEMORY_
#define __MEMORY_
void malloc_initialize(uint32_t);
uint32_t allocate_block32(void);
void free_block32(uint32_t address);
#endif
