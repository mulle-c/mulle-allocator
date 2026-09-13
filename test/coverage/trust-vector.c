#include <mulle-allocator/mulle-allocator.h>

#include <stdio.h>


static void   *my_reallocarray( void *block,
                                size_t n,
                                size_t size,
                                struct mulle_allocator *allocator)
{
   if( ! n)
      printf( "reallocarray zero\n");
   else
      if( n == (size_t) -1)
         printf( "reallocarray overflow\n");
      else
         printf( "reallocarray n=%zu\n", n);

   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( size);
   MULLE_C_UNUSED( allocator);
   return( (void *) 1);  // non-NULL, so there is no fail
}


static void   *my_calloc( size_t n,
                          size_t size,
                          struct mulle_allocator *allocator)
{
   if( ! n)
      printf( "calloc zero\n");
   else
      printf( "calloc n=%zu\n", n);

   MULLE_C_UNUSED( size);
   MULLE_C_UNUSED( allocator);
   return( (void *) 1);  // non-NULL, so there is no fail
}


int   main( void)
{
   struct mulle_allocator   a = mulle_allocator_stdlib;

   a.calloc       = my_calloc;
   a.reallocarray = my_reallocarray;

   // The library trusts the vector you install and must NOT pre-validate it:
   // a zero count and an overflowing count*size have to reach the vector
   // unchanged. Only the built-in default vector validates its own arguments.
   _mulle_allocator_reallocarray( &a, NULL, 0, 4);
   _mulle_allocator_reallocarray( &a, NULL, (size_t) -1, 2);
   _mulle_allocator_calloc( &a, 0, 4);
   return( 0);
}
