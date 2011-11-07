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

    uint32_t total_size = 0, ram_size;
    for(i =0 ; i < count; i++)
    {
        if(map[i].type == 1)
        {
            printf("a_low:%x, size:bytes %d (%d KB) (%d MB)\n",
                            map[i].base_addr_low,
                            map[i].base_length_low,
                            map[i].base_length_low>>10,
                            map[i].base_length_low>>20);
            if(ram_size <map[i].base_length_low)
                ram_size = map[i].base_length_low;
            total_size += map[i].base_length_low;            
        }        
    }
  
    asm volatile ("sti");    

    printf("\nTotal Ram Available : %d bytes (%d MB)\n", total_size, (total_size >> 20));
    initialise_virtual_paging(ram_size); 
    init_timer(500000);
    kernel_main();
    while(1);
}
