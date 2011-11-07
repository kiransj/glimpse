// paging.c -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#include <display.h>
#include "ram_manager.h"
#include "paging.h"
#include "util.h"
#include "memory.h"

PageDirectory currentDirectory, kernel_directory;
uint32_t switch_pd(PageDirectory directory)
{
	uint32_t cr3;
    uint32_t dir = (uint32_t)directory;    
    currentDirectory = directory;
	asm volatile("mov %%cr3, %%eax" : "=a"(cr3));
	asm volatile("mov %%eax, %%cr3" :: "a"(dir));
	return cr3;    
}

void disable_paging(void)
{
	uint32_t cr0;
	asm volatile("mov %%cr0, %%eax" : "=a"(cr0));
	cr0 &= ~(0x80000000);
	asm volatile("mov %%eax, %%cr0" :: "a"(cr0));
}

void enable_paging(void)
{
	uint32_t cr0;
	asm volatile("mov %%cr0, %%eax" : "=a"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %%eax, %%cr0" :: "a"(cr0));
}

/*
 * Page Format 
 * Present : 1
 * rw      : 1
 * user    : 1
 * accessed: 1
 * dirty   : 1
 * unused  : 7
 * frame   : 20
*/

void page_fault(registers_t regs);

PageDirectory PageDirectory_Create(void)
{
    uint32_t i =0 ;
    PageDirectory pd;
    pd = (PageDirectory)get_page();

    /*Assume page size is 4096*/
    for(i = 0; i < 1024; i++)
    {
        /*Set Not Preset bit*/
        pd[i] = 0x2; 
    }
    return pd;
}

void PageDirectory_MapAddress(PageDirectory pd, uint32_t from_address, uint32_t to_address)
{
    uint32_t page_number = (from_address >> 12) & 0x3FF;
    uint32_t page_table = (from_address >> 22) & 0x3FF;

    if(pd[page_table] == 0x2)
    {
        uint32_t table_address = get_page(), i;
        uint32_t *pt = (uint32_t*)table_address;

        /*Assume page size is 4096*/
        for(i = 0; i < 1024; i++)
        {
            /*Set Not Preset bit*/
            pt[i] = 0x2; 
        }
        /*Set the PRESENT and rw bit*/
        pd[page_table] = table_address | 0x3;
        pt[page_number] = (to_address & ~(0xFFF)) | 0x3;
    }
    else
    {
        uint32_t *pt = (uint32_t*)(pd[page_table] & 0xFFFFF000);
        pt[page_number] = (to_address & ~(0xFFF)) | 0x3;
    }
}
void PageDirectory_UnMapAddress(PageDirectory pd, uint32_t address)
{
    uint32_t page_number = (address >> 12) & 0x3FF;
    uint32_t page_table = (address >> 22) & 0x3FF;

    if(pd[page_table] == 0x2)
    {
        LOG_WARN("trying to unmap addressi %x which is not allocated", address);
    }
    else
    {
        uint32_t *pt = (uint32_t*)(pd[page_table] & 0xFFFFF000);
        pt[page_number] = 0x2;
    }
}

uint32_t get_mapped_page(void)
{
    uint32_t address = get_page();
    PageDirectory_MapAddress(currentDirectory, address, address);
    return address;
}

void free_mapped_page(uint32_t address)
{
    PageDirectory_UnMapAddress(currentDirectory, address);
    free_page(address);
}

void initialise_virtual_paging(uint32_t ram_size)
{    
    PageDirectory page_directory;
    uint32_t address = 0;
    
    initialize_ram(ram_size);
    page_directory = PageDirectory_Create();

    /*Map the first One MB*/
    for(address = 0; address < 0x100000; address += get_page_size())
    {
        PageDirectory_MapAddress(page_directory, address, address);
    }

    /*Now Map the allocated memory*/
    for(address = 0x100000; check_if_address_allocated(address); address += get_page_size())
    {
        PageDirectory_MapAddress(page_directory, address, address);
    }

    kernel_directory = page_directory;

    register_interrupt_handler(14, page_fault);
    switch_pd(page_directory);    
    enable_paging();

    malloc_initialize();
    return;
}


void page_fault(registers_t regs)
{
    // The faulting address is stored in the CR2 register.
    uint32_t faulting_address;

    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    
    // The error code gives us details of what happened.
    int present   = !(regs.err_code & 0x1); // Page not present
    int rw = regs.err_code & 0x2;           // Write operation?
    int us = regs.err_code & 0x4;           // Processor was in user-mode?
    int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = regs.err_code & 0x10;          // Caused by an instruction fetch?

    // Output an error message.
    LOG_ERROR("Page fault! ( ");
    if (present) {LOG_ERROR(" NOT present ");}
    if (rw) {LOG_ERROR(" read-only ");}
    if (us) {LOG_ERROR("user-mode ");}
    if (reserved) {LOG_ERROR("reserved ");}
    if (id) {LOG_ERROR("instructionFetch ");}
    LOG_ERROR(") at %x", faulting_address);

    asm volatile ("cli");
    while(1);
}

