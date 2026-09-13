#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handle_sigabrt(int sig)
{
   MULLE_C_UNUSED( sig);
   // sizeof( int) * (size_t) -1 overflows the heap fallback size. That is a
   // programmer error and must hard abort instead of allocating a wrapped,
   // undersized buffer.
   printf("Expected crash occurred.\n");
   exit(0);
}


int  main( void)
{
   signal( SIGABRT, handle_sigabrt);

   mulle_alloca_do( buf, int, (size_t) -1)
   {
      buf[ 0] = 1;
   }
   return( -1);
}
