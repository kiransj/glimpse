#include <util.h>
#include <display.h>
#include <descriptor_tables.h>
int kernel_main(void)
{  
    printf("\n\n --------> IN Main <-----------\n");

    asm volatile ("int $0x3");
    asm volatile ("int $0x4");
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

   kernel_main();
}
