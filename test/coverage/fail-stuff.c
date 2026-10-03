#include <mulle-allocator/mulle-allocator.h>

#include <stdio.h>
#include <stdlib.h>



static void   *fail_calloc( size_t n, size_t size, struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( allocator);
   MULLE_C_UNUSED( n);
   MULLE_C_UNUSED( size);
   return( NULL);
}


static void   *fail_reallocarray( void *block, size_t n, size_t size, struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( allocator);
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( n);
   MULLE_C_UNUSED( size);
   return( NULL);
}


static void   fail_free( void *block, struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( allocator);
   MULLE_C_UNUSED( block);
}


// `fail` is terminal (MULLE_C_NO_RETURN): a handler that returns is undefined
// behavior and clang emits an invalid opcode. Terminate after the first failure,
// like test/fails/fail-realloc.c does.
MULLE_C_NO_RETURN
void   fail_fail( struct mulle_allocator *p, void *block, size_t size)
{
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( p);
   printf( "failed for %zd bytes\n", size);
   exit( 0);
}


struct mulle_allocator   mulle_fail_allocator =
{
   .calloc       = fail_calloc,
   .reallocarray = fail_reallocarray,
   .free         = fail_free
};


int  main( void)
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