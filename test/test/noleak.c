#include <mulle-allocator/mulle-allocator.h>
#include <mulle-test-allocator/mulle-test-allocator.h>


static void  run_test( void)
{
   malloc( 1848);  // a leak we can't detect
}


int  main( int argc, char *argv[])
{
   mulle_test_allocator_initialize();
   mulle_default_allocator = mulle_test_allocator;
   {
      run_test();
   }
   mulle_test_allocator_reset();
   return( 0);
}
