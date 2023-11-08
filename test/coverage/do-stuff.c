#include <mulle-allocator/mulle-allocator.h>

#include <string.h>
#include <stdio.h>


static int  test( struct mulle_allocator *allocator)
{
   void  *p;
   char  *s;
   char  *t;

   p = mulle_allocator_malloc( allocator, 10);
   if( ! p)
      return( -1);
   p = mulle_allocator_realloc( allocator, p, 20);
   if( ! p)
      return( -1);

   mulle_allocator_free( allocator, p);


   if( mulle_allocator_is_stdlib_allocator( allocator))
      printf( "stdlib\n");
   else
      printf( "not stdlib\n");

   /**/

   p = mulle_allocator_calloc( allocator, 1, 10);
   if( ! p)
      return( -1);

   p = mulle_allocator_realloc_strict( allocator, p, 20);
   if( ! p)
      return( -1);

   p = mulle_allocator_realloc_strict( allocator, p, 0);
   if( p)
      return( -1);

   /**/

   t = "VfL Bochum 1848";
   s = mulle_allocator_strdup( allocator, t);
   printf( "%s\n", s);
   mulle_allocator_free( allocator, s);

   return( 0);
}


int  main( int argc, char *argv[])
{
   int  rc;

   rc = 0;
   rc |= test( NULL);
   rc |= test( &mulle_default_allocator);
   rc |= test( &mulle_stdlib_allocator);
   rc |= test( &mulle_stdlib_nofree_allocator);

   _mulle_allocator_invalidate( &mulle_default_allocator);

   return( rc);
}