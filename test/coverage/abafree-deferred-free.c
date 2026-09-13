#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <stdlib.h>


static int   freed;


//
// The allocator's own free. Proves that the deferred free adapter reaches it
// with the owning allocator.
//
static void   test_free( void *block, struct mulle_allocator *allocator)
{
   if( block && allocator)
      ++freed;
}


//
// A reclaimer of the exact abafree type: it performs the deferred free at
// once. No cast is used anywhere.
//
static int   test_abafree( void *aba,
                           mulle_allocator_deferred_free_t free,
                           void *block,
                           void *owner)
{
   MULLE_C_UNUSED( aba);
   free( block, owner);
   return( 0);
}


int  main( void)
{
   struct mulle_allocator   a;
   char                     block[ 1];

   a         = mulle_allocator_default;
   a.free    = test_free;
   a.abafree = test_abafree;
   a.aba     = NULL;

   mulle_allocator_abafree( &a, block);

   printf( "%d\n", freed);
   return( 0);
}
