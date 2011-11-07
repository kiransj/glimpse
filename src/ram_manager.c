#include <ram_manager.h>
#include <display.h>
#include "memory.h"
extern uint32_t end;

const uint32_t initial_ram_offset = 0x100000;

uint32_t num_of_pages;
uint32_t *page_bit_map;


inline uint32_t get_page_size()
{
    return 0x1000;
}
static void set_bit(uint32_t address)
{
    if(address >= initial_ram_offset)
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
            LOG_ERROR("bit_number (%d) > num_of_pages (%d)\n", page_number, num_of_pages);
            asm volatile("cli");
            while(1);
        }
    }
    else
    {
        LOG_ERROR("address %x is less than initial_ram_offset %x\n", address, initial_ram_offset);
        asm volatile("cli");
        while(1);
    }    
}
static void unset_bit(uint32_t address)
{
    uint32_t page_number = (address - initial_ram_offset) >> 12;
    if(page_number < num_of_pages)
    {
        uint32_t index =  page_number >> 5;
        uint32_t offset = page_number & 0x1F;

        if((page_bit_map[index] & ~(1 << offset)) == 0)
        {
            LOG_ERROR("address %x is not allocated at all", address);
        }
        else
        {    
            page_bit_map[index] &= ~((1 << offset));
        }
    }
    else
    {
        printf("bit_number (%d) > num_of_pages (%d)\n", page_number, num_of_pages);
        asm volatile("cli");
        while(1);
    }
}
uint32_t check_if_address_allocated(uint32_t address)
{
    uint32_t page_number = (address - initial_ram_offset) >> 12;
    if(page_number < num_of_pages)
    {
        uint32_t index =  page_number >> 5;
        uint32_t offset = page_number & 0x1F;
        return (page_bit_map[index] & ( 1 << offset));
    }
    else
    {
        return 0; 
    }
}
uint32_t get_page(void)
{
    uint32_t num_of_index = num_of_pages >> 2, i;

    for(i = 0; i < num_of_index; i++)
    {        
        if((page_bit_map[i] & 0xFFFFFFFF) != 0xFFFFFFFF)
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
    }
    return -1;
}

void free_page(uint32_t address)
{
    if((address & 0xFFF) != 0)
    {
        LOG_WARN("address is not aligned to page, proceeding anyways to free the page");

    }
    unset_bit(address);
}

void initialize_ram(uint32_t ram_size)    
{
    int bit_map_size;
    uint32_t current_ram_offset;

    /*Get the amount of memory used till now
     * end variable is defined in linker.ld*/
    current_ram_offset = ((uint32_t)&end + 0x1000 - 1) & ~(0x3FF);
    num_of_pages = (ram_size >> 12);

    /*bit map size in bytes (NOT WORD)*/
    bit_map_size = num_of_pages>>3;
    /*Allocate memory for page_bit_map*/
    page_bit_map = (uint32_t*)current_ram_offset;

    /* Increment the current ram offset as memory is 
     * allocated to page_bit_map*/
    while(bit_map_size > 0)
    {
        current_ram_offset += 0x1000;
        bit_map_size -= 0x1000;
    }
    bit_map_size = num_of_pages>>3;
    memset(page_bit_map, 0, bit_map_size); 

    malloc_initialize(current_ram_offset);
    current_ram_offset += 0x1000;
    /*Set the all the bits of ram that are currently used*/
    uint32_t tmp = initial_ram_offset;
    while(tmp < current_ram_offset)
    {
        set_bit(tmp);
        tmp += 0x1000;
    }

    return;
}
