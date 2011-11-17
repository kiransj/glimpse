#ifndef __MY_MALLOC_
#define __MY_MALLOC_

#include <util.h>
void kmalloc_initialize(const uint32_t addre, const uint32_t size);
void kfree(void *addr);
void* kmalloc(const uint32_t size);
#endif
