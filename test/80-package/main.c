#include <mulle-allocator/mulle-allocator.h>


// happy if it compiles...
int   main( void)
{
   void  *p;

   p = mulle_malloc( 100);
   mulle_free( p);
   return( 0);
}
