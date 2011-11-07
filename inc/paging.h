// paging.h -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#ifndef PAGING_H
#define PAGING_H

#include "util.h"
#include "isr.h"


typedef unsigned int *PageDirectory;

typedef struct __page_directory
{
    PageDirectory pd;
    uint32_t heap_allocate_address;
    uint32_t num_of_pages_freed;
}*Page_Directory;

/**
   Sets up the environment, page directories etc and
   enables paging.
**/
void initialise_virtual_paging(uint32_t ram_size);

Page_Directory PageDirectory_Create(void);
void PageDirectory_MapAddress(Page_Directory pd, uint32_t from_address, uint32_t to_address);
uint32_t get_mapped_page(uint32_t size);
void free_mapped_page(uint32_t address, uint32_t size);
#endif
