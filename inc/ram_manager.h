#ifndef _RAM_MANAGER_
#define _RAM_MANAGER_
#include <util.h>
void initialize_ram(uint32_t ram_size);
uint32_t check_if_address_allocated(uint32_t address);
uint32_t get_page(void);
inline uint32_t get_page_size();
void free_page(uint32_t address);
#endif
