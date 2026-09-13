#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <stdlib.h>


MULLE_C_NO_RETURN
static void   fail( struct mulle_allocator *allocator, void *unused, size_t ignored)
{
   MULLE_C_UNUSED( allocator);
   MULLE_C_UNUSED( ignored);
   MULLE_C_UNUSED( unused);
   printf( "OK\n");
   exit( 0);
}


int  main( void)
{
   mulle_default_allocator.fail = fail;
   mulle_allocator_reallocarray( NULL, NULL, 1, (size_t) -1);  // just leak
   return( -1);
}
