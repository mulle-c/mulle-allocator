#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>


void handle_sigabrt(int sig)
{
   MULLE_C_UNUSED( sig);
   // Growing a stack-backed alloca buffer by an overflowing count must route
   // through mulle_allocarray and fail there, not compute a wrapped size.
   printf( "Expected crash occurred.\n");
   exit( 0);
}


int  main( void)
{
   signal( SIGABRT, handle_sigabrt);

   mulle_alloca_do( buf, int, 4)
   {
      mulle_alloca_do_realloc( buf, (size_t) -1);
      buf[ 0] = 1;
   }
   return( -1);
}
