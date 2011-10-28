#include <util.h>
#include <display.h>

int kernel_main(void)
{    
    printf("first %c %s:%u:%d", '>', "printf", 1000, -41234);
    return 0;
}

void kernel_entry( void* mbd, unsigned int magic )
{
   UNUSED_PARAMETER(mbd);

   clear_screen(); 
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
