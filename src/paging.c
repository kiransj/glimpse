// paging.c -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#include <display.h>
#include "paging.h"
#include "kheap.h"

uint32_t switch_pd(uint32_t dir)
{
	uint32_t cr3;
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

void initialize_ram(uint32_t ram_size);
static uint32_t get_page(void);

void initialise_virtual_paging(uint32_t ram_size)
{
    
    uint32_t *page_directory;
    uint32_t *page_table, i, address = 0;

    initialize_ram(ram_size);
    
    page_directory = (uint32_t*)get_page();

    page_table = (uint32_t*)get_page();
    page_directory[0] = ((uint32_t)page_table) | 0x3;

    for(i = 0, address = 0; i < 1024; i++)
    {
        page_table[i] = address | 0x3;
        address += 0x1000;
    }

    /*Unsed Pagetable*/
    for(i = 1; i < 1024; i++)
    {
        page_directory[i] = 0x2;
    }

    register_interrupt_handler(14, page_fault);
    switch_pd((uint32_t)page_directory);
    enable_paging();
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
    printf("Page fault! ( ");
    if (present) {printf(" NOT present ");}
    if (rw) {printf(" read-only ");}
    if (us) {printf("user-mode ");}
    if (reserved) {printf("reserved ");}
    if (id) {printf("instructionFetch ");}
    printf(") at %x", faulting_address);
    while(1);
}


extern uint32_t end;

uint32_t current_ram_offset;
const uint32_t initial_ram_offset = 0x100000;

uint32_t num_of_pages;
uint32_t *page_bit_map;

static void set_bit(uint32_t address)
{
    uint32_t page_number = (address - initial_ram_offset) >> 12;
    if(page_number < num_of_pages)
    {
        uint32_t index =  page_number >> 5;
        uint32_t offset = page_number & 0x1F;

        page_bit_map[index] |= (1 << offset);
    }
    else
    {
        printf("bit_number (%d) > num_of_pages (%d)\n", page_number, num_of_pages);
        asm volatile("cli");
        while(1);
    }
}

static uint32_t get_page(void)
{
    uint32_t num_of_index = num_of_pages >> 2, i;

    for(i = 0; i < num_of_index; i++)
    {
        uint32_t flag = 1, j;
        for(j = 0; j < 32; j++)
        {
            if((page_bit_map[i] & (flag << j)) == 0)
            {
                page_bit_map[i] |= flag << j;
                return initial_ram_offset + (((i << 5) + j) << 12);
            }
        }
    }
    return -1;
}

void initialize_ram(uint32_t ram_size)    
{
    int bit_map_size;
    current_ram_offset = ((uint32_t)&end + 0x1000 - 1) & ~(0x3FF);
    num_of_pages = (ram_size >> 12);
    printf("Number of physical pages : %x\n", num_of_pages);

    /*bit map size in bytes (NOT WORD)*/
    bit_map_size = num_of_pages>>3;
    printf("bit_map_size : %x\n", bit_map_size);
    page_bit_map = (uint32_t*)current_ram_offset;

    while(bit_map_size > 0)
    {
        printf("\none bit_map_size : %x\n", bit_map_size);
        current_ram_offset += 0x1000;
        bit_map_size -= 0x1000;
    }
    bit_map_size = num_of_pages>>3;
    memset(page_bit_map, 0, bit_map_size); 
    uint32_t tmp = initial_ram_offset;
    while(tmp < current_ram_offset)
    {
        set_bit(tmp);
        tmp += 0x1000;
    }
}
