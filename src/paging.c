// paging.c -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#include <display.h>
#include "ram_manager.h"
#include "paging.h"
#include "util.h"
#include "malloc.h"
#include "asm.h"
#include "schedule.h"
typedef unsigned int *PageDirectory;

struct __page_directory
{
    PageDirectory pd;
    uint32_t heap_allocate_address;
    uint32_t num_of_pages_freed;
};
Page_Directory currentDirectory, kernel_directory;


uint32_t switch_pd(Page_Directory directory)
{
	uint32_t cr3;
    uint32_t dir = (uint32_t)directory->pd;
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

#define PAGE_PRESENT        0x1
#define PAGE_RW             0x2
#define PAGE_USER_MODE      0x4

void page_fault(registers_t regs);

Page_Directory PageDirectory_Create(void)
{
    Page_Directory PD = (Page_Directory)kmalloc(sizeof(struct __page_directory));
    PD->pd = (PageDirectory)get_page();
    PD->heap_allocate_address = PD->num_of_pages_freed = 0;
    memset(PD->pd, 0, 4096);
    return PD;
}

void PageDirectory_MapAddress(Page_Directory PD, uint32_t logical_address, uint32_t physical_address, bool user)
{
    uint32_t flags = (PAGE_PRESENT | PAGE_RW | (user? PAGE_USER_MODE : 0));
    uint32_t page_number = (logical_address >> 12) & 0x3FF;
    uint32_t page_table = (logical_address >> 22) & 0x3FF;

    if(PD->pd[page_table] == 0x0)
    {
        uint32_t table_address = get_page();
        uint32_t *pt = (uint32_t*)table_address;
        /*Set the PRESENT and rw bit*/
        PD->pd[page_table] = table_address | flags;
        PageDirectory_MapAddress(PD, table_address, table_address, false);

    
        memset(pt, 0, 4096);
        /*Add the currently allocated page back to PageTable*/
        pt[page_number] = (physical_address & ~(0xFFF)) | flags;
    }
    else
    {
        uint32_t *pt = (uint32_t*)(PD->pd[page_table] & 0xFFFFF000);
        pt[page_number] = (physical_address & ~(0xFFF)) | flags;
    }
}
uint32_t PageDirectory_UnMapAddress(Page_Directory PD, uint32_t address)
{
    uint32_t phy_address = 0;
    uint32_t page_number = (address >> 12) & 0x3FF;
    uint32_t page_table = (address >> 22) & 0x3FF;

    if(PD->pd[page_table] == 0x0)
    {
        LOG_WARN("trying to unmap addressi %x which is not allocated", address);
    }
    else
    {
        uint32_t *pt = (uint32_t*)(PD->pd[page_table] & 0xFFFFF000);
        phy_address = pt[page_number] & ~(0xFFF);
        pt[page_number] = 0x0;
        PD->num_of_pages_freed++;
    }
    return phy_address;
}

uint32_t get_mapped_page(uint32_t size)
{
    uint32_t logical_address;
    if((size & 0xFFF) == 0)
    {
        logical_address = currentDirectory->heap_allocate_address;
        LOG_INFO("logical_address : %x", logical_address);
        while(size > 0)
        {
            uint32_t phy_address = get_page();
            PageDirectory_MapAddress(currentDirectory, currentDirectory->heap_allocate_address, phy_address, false);
            currentDirectory->heap_allocate_address += 0x1000;
            size -= 0x1000;
        }
    }
    else
    {
        LOG_ERROR("size should be multiple for 4096");
        logical_address = 0;
    }
    return logical_address;
}

void free_mapped_page(uint32_t address, uint32_t size)
{
    FN_ENTRY();
    if((size & 0xFFF) == 0)
    {
        uint32_t phyaddress;
        while(size > 0)
        {
            phyaddress = PageDirectory_UnMapAddress(currentDirectory, address);
            free_page(phyaddress);
            size -= 0x1000;
            address += 0x1000;
        }
    }
    else
    {
        LOG_ERROR("size should be multiple for 4096");
    }
    FN_EXIT();
    return;
}

void initialise_virtual_paging(uint32_t ram_size)
{
    Page_Directory page_directory;
    uint32_t address = 0;

    initialize_ram(ram_size);
    page_directory = PageDirectory_Create();

    /*Map the first One MB*/
    for(address = 0; address < 0x100000; address += get_page_size())
    {
        PageDirectory_MapAddress(page_directory, address, address, false);
    }

    /*Now Map the allocated memory*/
    for(address = 0x100000; check_if_address_allocated(address); address += get_page_size())
    {
        PageDirectory_MapAddress(page_directory, address, address, false);
    }

    kernel_directory = page_directory;
    kernel_directory->heap_allocate_address = 0x2000000;

    register_interrupt_handler(14, page_fault);
    switch_pd(page_directory);
    enable_paging();

    return;
}


void page_fault(registers_t regs)
{
    // The faulting address is stored in the CR2 register.
	CLEAR_INTERRUPT();
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

    LOG_ERROR("Page fault from task : %d", get_pid());    
    while(1);
}

