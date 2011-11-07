#include <util.h>
#include <display.h>
#include <descriptor_tables.h>
#include <timer.h>
#include <multiboot.h>
#include <paging.h>

#define GET_ESP(x)  asm volatile("mov %%esp, %0": "=r"(x))
int kernel_main(void)
{  
    printf("\n\n --------> IN Main <----------- %d\n", 0);
    uint32_t *address =(uint32_t*)0x500000;

    address[0] = 1;

    return 0;
}



extern uint32_t end;

void page_fault_1(registers_t regs)
{
    printf("\nPage fault");

    while(1);
}

uint32_t switch_pd(uint32_t dir)
{
	uint32_t cr3;

    FN_ENTRY();
	asm volatile("mov %%cr3, %%eax" : "=a"(cr3));
	asm volatile("mov %%eax, %%cr3" :: "a"(dir));

    FN_EXIT();
	return cr3;    
}

void enable_paging(void)
{
	uint32_t cr0;
    FN_ENTRY();
	asm volatile("mov %%cr0, %%eax" : "=a"(cr0));
	cr0 |= 0x80000000;
    printf("\ncr0 : %x\n", cr0);
	asm volatile("mov %%eax, %%cr0" :: "a"(cr0));
    FN_EXIT();
}
void initialise_virtual_paging(void)
{
    uint32_t *page_directory;
    uint32_t *page_table, i, address = 0;
    uint32_t ram_offset = ((uint32_t)&end + 0x1000) & ~(0xFFF);

    page_directory = (uint32_t*)ram_offset;
    ram_offset += 0x1000;

    page_table = (uint32_t*)ram_offset;
    page_directory[0] = ram_offset | 0x3;
    ram_offset += 0x1000;

    for(i = 0, address = 0; i < 1024; i++)
    {
        page_table[i] = address | 0x3;
        address += 0x1000;
    }


    for(i = 1; i < 1024; i++)
    {
        page_directory[i] = 0x2;
    }

    register_interrupt_handler(14, page_fault_1);
    switch_pd((uint32_t)page_directory);
    enable_paging();
}
struct memory_map
{
    uint32_t size;
    uint32_t base_addr_low;
    uint32_t base_addr_high;
    uint32_t base_length_low;
    uint32_t base_length_high;
    uint32_t type;
};
void kernel_entry(struct multiboot *mbd, uint32_t esp)
{
    uint32_t current_stack = 0;
    UNUSED_PARAMETER(mbd);
    UNUSED_PARAMETER(esp);
    clear_screen(); 
    
    init_descriptor_tables();    

    GET_ESP(current_stack);

    printf("\nmem_lower : %x (%d KB), mem_upper : %x (%d KB)\n",mbd->mem_lower, mbd->mem_lower, mbd->mem_upper, mbd->mem_upper);
    uint32_t count = mbd->mmap_length/sizeof(struct memory_map), i;
    struct memory_map *map = (struct memory_map *)mbd->mmap_addr;

    uint32_t total_size = 0;
    for(i =0 ; i < count; i++)
    {
        if(map[i].type == 1)
        {
            printf("a_low:%x, size:bytes %d (%d KB) (%d MB)\n",
                            map[i].base_addr_low,
                            map[i].base_length_low,
                            map[i].base_length_low>>10,
                            map[i].base_length_low>>20);
            total_size += map[i].base_length_low;            
        }        
    }
  
    asm volatile ("sti");    

    printf("\nTotal Ram Available : %d bytes (%d MB)\n", total_size, (total_size >> 20));
    initialise_virtual_paging();    
    init_timer(500000);
    kernel_main();
    while(1);
}
