#include <mulle-allocator/mulle-allocator.h>
#include <mulle-test-allocator/mulle-test-allocator.h>


static void  run_test( void)
{
   void  *p;

   p = mulle_malloc( 1848);  // just leak
   mulle_free( p);
   mulle_free( p);
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
