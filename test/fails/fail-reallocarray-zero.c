#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <stdlib.h>


MULLE_C_NO_RETURN
static void   fail( struct mulle_allocator *allocator, void *unused, size_t ignored)
{
   printf( "OK\n");
   exit( 0);
}


int  main( int argc, char *argv[])
{
   // passing n == 0 to the non-strict reallocarray is a programming error
   // (use reallocarray_strict to free and get NULL back), so this must
   // trigger the fail handler in all builds, not just debug
   mulle_default_allocator.fail = fail;
   mulle_allocator_reallocarray( NULL, NULL, 0, sizeof( int));
   return( -1);
}