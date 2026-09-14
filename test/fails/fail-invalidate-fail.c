#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


void handle_sigabrt(int sig)
{
   MULLE_C_UNUSED( sig);
   printf( "Expected crash occurred.\n");
   exit( 0);
}


int  main( void)
{
   struct mulle_allocator   a;

   signal( SIGABRT, handle_sigabrt);

   _mulle_allocator_invalidate( &a);
   a.fail( &a, NULL, 0);

   return( -1);
}
