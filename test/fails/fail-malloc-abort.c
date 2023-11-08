#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handle_sigabrt(int sig)
{
   // Handle the SIGABRT signal here.
   // For example, you could print a message and exit with a zero status.
   printf("Expected crash occurred.\n");
   exit(0);
}



int  main( int argc, char *argv[])
{
   signal( SIGABRT, handle_sigabrt);

   mulle_malloc( -1);  // just cras
   return( -1);
}
