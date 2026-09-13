#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <stdlib.h>


MULLE_C_NO_RETURN
static void   fail( struct mulle_allocator *allocator, void *block, size_t size)
{
   MULLE_C_UNUSED( allocator);
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( size);
   printf( "OK\n");
   exit( 0);
}


int  main( void)
{
   // passing n == 0 to the non-strict reallocarray is a programming error. It
   // is still routed through the allocator's fail vector, so a custom handler
   // observes it. The handler is terminal: it must not return.
   mulle_allocator_set_fail( &mulle_allocator_default, fail);

   mulle_allocator_reallocarray( NULL, NULL, 0, sizeof( int));
   return( -1);
}
