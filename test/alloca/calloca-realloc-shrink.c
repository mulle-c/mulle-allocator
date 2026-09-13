#include <mulle-allocator/mulle-allocator.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>


int   main( void)
{
   mulle_calloca_do( buf, int, 4)
   {
      mulle_calloca_do_realloc( buf, 2);

      // shrinking within the stack-resident buffer must still record the new
      // element count
      printf( "%lu\n", (unsigned long) (uintptr_t) buf__count);
   }
   return( 0);
}
