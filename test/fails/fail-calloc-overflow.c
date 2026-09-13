#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handle_sigabrt(int sig)
{
   MULLE_C_UNUSED( sig);
   // overflow is a programmer error: it must hard abort and never be retried
   printf("Expected crash occurred.\n");
   exit(0);
}


int  main( void)
{
   signal( SIGABRT, handle_sigabrt);

   mulle_calloc( 2, (size_t) -1);  // 2 * SIZE_MAX overflows, must cras
   return( -1);
}
