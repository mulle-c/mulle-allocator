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


int   main( void)
{
   struct mulle_allocator   a = mulle_allocator_stdlib;

   // 2 * SIZE_MAX overflows. The default calloc vector validates this itself
   // and routes the programmer error through the allocator's fail vector.
   a.fail = fail;
   _mulle_allocator_calloc( &a, 2, (size_t) -1);
   return( -1);
}
