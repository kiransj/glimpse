#include <util.h>
#include <display.h>
#include <descriptor_tables.h>
#include <multiboot.h>
#include <paging.h>
#include <schedule.h>
#include <asm.h>
#define GET_ESP(x)  asm volatile("mov %%esp, %0": "=r"(x))


extern uint32_t timer_ticks;

int kernel_main(void)
{
    FN_ENTRY();
    LOG_INFO("\n\n --------> IN Main <----------- ");
    uint32_t *address =(uint32_t*)get_mapped_page(sizeof(uint32_t) * 0x2000), i;

    LOG_INFO("address : %x",address);

    for(i = 0; i < 0x2000; i++)
    {
        LOG_WARN("%x", i);
        address[i] = 1;
    }

    free_mapped_page((uint32_t)address, sizeof(uint32_t) * 0x2000);

    LOG_INFO("done");
    FN_EXIT();
    return 0;
}

int my_thread_sleep(void)
{
    uint32_t num = 0;
    while(1)
    {
        sleep(10);
        if(num == 10)
            break;
        printf("Sleep:Thread Id : %d, %d\n", get_pid(), num++);
    }
    printf("Ending thread %d\n", get_pid());
    return 0;
}
int my_thread_nosleep(void)
{
    uint32_t num = 0;
    while(1)
    {
        sleep(100);
        printf("NoSleep:Thread Id : %d, %d\n", get_pid(), num++);
    }
    return 0;
}

int main_thread(void)
{
   sleep(1000);
   CLEAR_INTERRUPT();
   print_ktask_list();
   while(1);
   return 0;
}
void kernel_entry(struct multiboot *mbd, uint32_t esp)
{
	CLEAR_INTERRUPT();
    uint32_t current_stack = 0;
    UNUSED_PARAMETER(mbd);
    UNUSED_PARAMETER(esp);
    clear_screen();

    init_descriptor_tables();

    GET_ESP(current_stack);

    uint32_t count = mbd->mmap_length/sizeof(struct memory_map), i;
    struct memory_map *map = (struct memory_map *)mbd->mmap_addr;

    uint32_t total_size = 0, ram_size;
    for(i =0 ; i < count; i++)
    {
        if(map[i].type == 1)
        {
#if 0
            printf("a_low:%x, size:bytes %d (%d KB) (%d MB)\n",
                            map[i].base_addr_low,
                            map[i].base_length_low,
                            map[i].base_length_low>>10,
                            map[i].base_length_low>>20);
#endif
            if(ram_size <map[i].base_length_low)
                ram_size = map[i].base_length_low;
            total_size += map[i].base_length_low;
        }
    }

    initialise_virtual_paging(ram_size);
    initialize_scheduling();

    ENABLE_INTERRUPT();
    

    kthread_create(my_thread_nosleep, "thread2");
    kthread_create(my_thread_nosleep, "thread3");
    
    kthread_create(my_thread_sleep, "thread1");
    while(1)
    {
        printf("main thread : %d\n", get_pid());
        uint32_t wake_up = timer_ticks + 100;
        while(wake_up > timer_ticks);
    }
#if 0 
    kernel_main();
    while(1);
#endif
    return;
}
