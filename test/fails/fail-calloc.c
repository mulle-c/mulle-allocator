#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>


MULLE_C_NO_RETURN
static void   fail( void *unused, size_t ignored)
{
   printf( "OK\n");
   exit( 0);
}


int  main( int argc, char *argv[])
{
   mulle_default_allocator.fail = fail;
   mulle_calloc( 1, -1);  // just leak
   return( -1);
}
