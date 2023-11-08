#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <stdlib.h>


MULLE_C_NO_RETURN
static void   fail( struct mulle_allocator *allocaor, void *unused, size_t ignored)
{
   printf( "OK\n");
   exit( 0);
}


static void  *fail_realloc( void *p, size_t len, struct mulle_allocator *allocator)
{
   return( NULL);
}


int  main( int argc, char *argv[])
{
   mulle_default_allocator.realloc = fail_realloc;
   mulle_default_allocator.fail    = fail;
   mulle_strdup( "VfL Bochum 1848");
   return( -1);
}
