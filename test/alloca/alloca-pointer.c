#include <mulle-allocator/mulle-allocator.h>

#include <string.h>
#include <stdio.h>
#include <stdint.h>


struct foo
{
   int   a;
};


int  main( int argc, char *argv[])
{
   unsigned int   count = 3;
   struct foo     *p = (struct foo*) 0x20;

   // just a test that figured out that we had problems with pointers
   mulle_alloca_do( values, struct foo *, 3)
   {
      if( p == values[ 0])
         return( 1);
   }
   return( 0);
}
