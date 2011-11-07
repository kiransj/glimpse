// paging.h -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#ifndef PAGING_H
#define PAGING_H

#include "util.h"
#include "isr.h"



typedef struct __page_directory *Page_Directory;
/**
   Sets up the environment, page directories etc and
   enables paging.
**/
void initialise_virtual_paging(uint32_t ram_size);

uint32_t get_mapped_page(uint32_t size);
void free_mapped_page(uint32_t address);
#endif
