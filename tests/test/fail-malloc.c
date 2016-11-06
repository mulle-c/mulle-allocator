#include <mulle_allocator/mulle_allocator.h>
#include <mulle_test_allocator/mulle_test_allocator.h>


static void   fail( void *unused, size_t ignored)
{
   printf( "OK\n");
   exit( 0);
}


int  main( int argc, char *argv[])
{
   mulle_default_allocator.fail = fail;
   mulle_malloc( -1);  // just leak
   return( -1);
}
