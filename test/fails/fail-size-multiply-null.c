#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


void handle_sigabrt(int sig)
{
   MULLE_C_UNUSED( sig);
   // A NULL allocator means "the default allocator". On overflow the helper
   // must route through that allocator's fail vector and abort -- not
   // dereference the NULL allocator.
   printf( "Expected crash occurred.\n");
   exit( 0);
}


int  main( void)
{
   signal( SIGABRT, handle_sigabrt);

   (void) mulle_allocator_size_multiply( NULL, (size_t) -1, 2);
   return( -1);
}
