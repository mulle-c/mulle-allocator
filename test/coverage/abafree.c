#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <stdlib.h>


static int   placebo_abafree( void *aba, void (*free)( void *, void *), void *block, void *owner)
{
   printf( "%s %s\n", (char *) aba, (char *) block);
   return( 0);
}


int  main( int argc, char *argv[])
{
   mulle_default_allocator.abafree = placebo_abafree;
   mulle_default_allocator.aba     = "VfL Bochum";
   mulle_abafree( "1848");
   return( 0);
}
