// paging.h -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#ifndef PAGING_H
#define PAGING_H

#include "util.h"
#include "isr.h"


typedef unsigned int *PageDirectory;
/**
   Sets up the environment, page directories etc and
   enables paging.
**/
void initialise_virtual_paging(uint32_t ram_size);

PageDirectory PageDirectory_Create(void);
void PageDirectory_MapAddress(PageDirectory pd, uint32_t from_address, uint32_t to_address);
#endif
