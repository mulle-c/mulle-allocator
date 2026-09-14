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
   mulle_default_allocator.fail = fail;
   mulle_calloc( 0, 1);  // zero count is a programmer error
   return( -1);
}
