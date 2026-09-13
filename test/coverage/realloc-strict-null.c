#include <mulle-allocator/mulle-allocator.h>

#include <stdio.h>
#include <stdlib.h>


static int   freed_null;


static void   my_free( void *block, struct mulle_allocator *allocator)
{
   if( ! block)
      freed_null = 1;
   else
      free( block);

   MULLE_C_UNUSED( allocator);
}


int   main( void)
{
   struct mulle_allocator   a = mulle_allocator_stdlib;

   a.free = my_free;

   // strict with size zero frees and returns NULL. A NULL block must not be
   // handed to a custom free that may not tolerate NULL.
   if( _mulle_allocator_realloc_strict( &a, NULL, 0) != NULL)
      return( 1);
   if( _mulle_allocator_reallocarray_strict( &a, NULL, 0, 8) != NULL)
      return( 2);
   if( freed_null)
      return( 3);

   printf( "OK\n");
   return( 0);
}
