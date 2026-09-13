#include <mulle-allocator/mulle-allocator.h>

#include <stdio.h>
#include <stdlib.h>


MULLE_C_NO_RETURN
static void   fail( struct mulle_allocator *allocator, void *block, size_t size)
{
   MULLE_C_UNUSED( allocator);
   MULLE_C_UNUSED( block);
   printf( "%zu\n", size);
   exit( 0);
}


// an installed vector that can never satisfy a request
static void   *reallocarray_null( void *block,
                                  size_t n,
                                  size_t size,
                                  struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( n);
   MULLE_C_UNUSED( size);
   MULLE_C_UNUSED( allocator);
   return( NULL);
}


int   main( void)
{
   struct mulle_allocator   a = mulle_allocator_stdlib;

   a.reallocarray = reallocarray_null;
   a.fail         = fail;

   // the fail handler is promised the requested size in bytes, so 7 * 8 == 56
   mulle_allocator_reallocarray( &a, NULL, 7, 8);
   return( -1);
}
