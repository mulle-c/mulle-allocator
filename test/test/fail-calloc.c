#include <mulle-allocator/mulle-allocator.h>
#include <mulle-test-allocator/mulle-test-allocator.h>


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
