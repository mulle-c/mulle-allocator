#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <stdlib.h>


MULLE_C_NO_RETURN
static void   fail( struct mulle_allocator *allocaor, void *unused, size_t ignored)
{
   MULLE_C_UNUSED( allocaor);
   MULLE_C_UNUSED( ignored);
   MULLE_C_UNUSED( unused);
   printf( "OK\n");
   exit( 0);
}


static void  *fail_reallocarray( void *block, size_t n, size_t size, struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( allocator);
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( n);
   MULLE_C_UNUSED( size);
   return( NULL);
}


int  main( void)
{
   mulle_default_allocator.reallocarray = fail_reallocarray;
   mulle_default_allocator.fail         = fail;
   mulle_strdup( "VfL Bochum 1848");
   return( -1);
}
