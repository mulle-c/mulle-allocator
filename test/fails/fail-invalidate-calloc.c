#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


void handle_sigabrt(int sig)
{
   MULLE_C_UNUSED( sig);
   // _mulle_allocator_invalidate replaced every vector with a typed abort
   // stub, so using the invalidated allocator must hard-abort.
   printf( "Expected crash occurred.\n");
   exit( 0);
}


int  main( void)
{
   struct mulle_allocator   a;

   signal( SIGABRT, handle_sigabrt);

   _mulle_allocator_invalidate( &a);
   a.calloc( 1, 1, &a);

   return( -1);
}
