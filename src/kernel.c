#include <util.h>
#include <display.h>
#include <descriptor_tables.h>
#include <timer.h>
int kernel_main(void)
{  
    printf("\n\n --------> IN Main <----------- %d\n", 0);

    return 0;
}

void kernel_entry( void* mbd, unsigned int magic )
{
   UNUSED_PARAMETER(mbd);

   clear_screen(); 

   init_descriptor_tables();
   
   if ( magic != 0x2BADB002 )
   {
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
       printf("\nMagic Number did not match :( !!!");
       return;
   }    
   asm volatile ("int $0x3");
   asm volatile ("int $0x4");
   asm volatile("sti");
   init_timer(10000);
   kernel_main();
}
