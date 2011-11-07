// paging.h -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#ifndef PAGING_H
#define PAGING_H

#include "util.h"
#include "isr.h"


/**
   Sets up the environment, page directories etc and
   enables paging.
**/
void initialise_virtual_paging(uint32_t ram_size);


/**
   Handler for page faults.
**/
void page_fault(registers_t regs);

#endif
