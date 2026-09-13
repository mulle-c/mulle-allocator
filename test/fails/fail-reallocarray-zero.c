#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


void handle_sigabrt(int sig)
{
   MULLE_C_UNUSED( sig);
   // passing n == 0 to the non-strict reallocarray is a programming error
   // (use reallocarray_strict to free and get NULL back). Programmer errors
   // are not recoverable, so a custom fail handler is not consulted and the
   // call hard-aborts.
   printf( "Expected crash occurred.\n");
   exit( 0);
}


int  main( void)
{
   signal( SIGABRT, handle_sigabrt);

   mulle_allocator_reallocarray( NULL, NULL, 0, sizeof( int));
   return( -1);
}
