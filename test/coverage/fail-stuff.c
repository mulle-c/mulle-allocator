#include <mulle-allocator/mulle-allocator.h>

#include <stdio.h>



static void   *fail_calloc( size_t n, size_t size, struct mulle_allocator *allocator)
{
   return( NULL);
}


static void   *fail_realloc( void *block, size_t size, struct mulle_allocator *allocator)
{
   return( NULL);
}


static void   fail_free( void *block, struct mulle_allocator *allocator)
{
}


static int   fail_aba( void *block, void (*f)( void *, void *), void *a, void *b)
{
   return( -1);
}


// ARGH! CAN'T DO THAT ANYMORE, BECAUSE CLANG PLACES AN INVALID
// OPCODE THERE TO "CATCH" THE ERROR.
// MULLE_C_NO_RETURN
void   fail_fail( struct mulle_allocator *p, void *block, size_t size)
{
   printf( "failed for %zd bytes\n", size);
   // returns intentionally
}


struct mulle_allocator   mulle_fail_allocator =
{
   .calloc  = fail_calloc,
   .realloc = fail_realloc,
   .free    = fail_free
};


int  main( int argc, char *argv[])
{
   mulle_allocator_set_fail( &mulle_fail_allocator, (void *) fail_fail);
   mulle_allocator_set_aba( &mulle_fail_allocator, (void *) fail_fail, NULL);
   mulle_allocator_malloc( &mulle_fail_allocator, 10);
   mulle_allocator_realloc( &mulle_fail_allocator, NULL, 20);
   mulle_allocator_realloc_strict( &mulle_fail_allocator, NULL, 30);
   mulle_allocator_strdup( &mulle_fail_allocator, "VfL Bochum 1848");
   mulle_allocator_abafree( &mulle_fail_allocator, NULL);

   if( mulle_allocator_is_stdlib_allocator( &mulle_fail_allocator))
      printf( "stdlib\n");
   else
      printf( "not stdlib\n");

   return( 0);
}