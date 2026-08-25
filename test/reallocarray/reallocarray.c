#include <mulle-allocator/mulle-allocator.h>

#include <stdio.h>


static int  test_reallocarray( struct mulle_allocator *allocator)
{
   int    *p;
   size_t i;

   // malloc-like: block == NULL
   p = mulle_allocator_reallocarray( allocator, NULL, 4, sizeof( int));
   if( ! p)
      return( -1);
   for( i = 0; i < 4; i++)
      p[ i] = (int) i + 1;
   for( i = 0; i < 4; i++)
      if( p[ i] != (int) i + 1)
         return( -2);

   // grow: old contents must be preserved
   p = mulle_allocator_reallocarray( allocator, p, 8, sizeof( int));
   if( ! p)
      return( -3);
   for( i = 0; i < 4; i++)
      if( p[ i] != (int) i + 1)
         return( -4);
   for( i = 4; i < 8; i++)
      p[ i] = (int) i + 1;

   // shrink
   p = mulle_allocator_reallocarray( allocator, p, 2, sizeof( int));
   if( ! p)
      return( -5);
   if( p[ 0] != 1 || p[ 1] != 2)
      return( -6);

   mulle_allocator_free( allocator, p);
   return( 0);
}


static int  test_reallocarray_default( void)
{
   int    *p;
   size_t i;

   // convenience wrapper uses the default allocator
   p = mulle_reallocarray( NULL, 4, sizeof( int));
   if( !p)
      return( -1);
   for( i = 0; i < 4; i++)
      p[ i] = (int) i + 1;
   for( i = 0; i < 4; i++)
      if( p[ i] != (int) i + 1)
         return( -2);

   // grow: old contents must be preserved
   p = mulle_reallocarray( p, 8, sizeof( int));
   if( !p)
      return( -3);
   for( i = 0; i < 4; i++)
      if( p[ i] != (int) i + 1)
         return( -4);
   for( i = 4; i < 8; i++)
      p[ i] = (int) i + 1;

   // shrink
   p = mulle_reallocarray( p, 2, sizeof( int));
   if( !p)
      return( -5);
   if( p[ 0] != 1 || p[ 1] != 2)
      return( -6);

   mulle_allocator_free( NULL, p);
   return( 0);
}


static int  test_reallocarray_strict( struct mulle_allocator *allocator)
{
   int    *p;
   size_t i;

   p = mulle_allocator_reallocarray_strict( allocator, NULL, 4, sizeof( int));
   if( ! p)
      return( -1);
   for( i = 0; i < 4; i++)
      p[ i] = (int) i + 1;

   // n == 0: frees block and returns NULL
   if( mulle_allocator_reallocarray_strict( allocator, p, 0, sizeof( int)))
      return( -2);

   // size == 0: frees block and returns NULL
   p = mulle_allocator_reallocarray_strict( allocator, NULL, 4, sizeof( int));
   if( ! p)
      return( -3);
   if( mulle_allocator_reallocarray_strict( allocator, p, 4, 0))
      return( -4);

   return( 0);
}


static int  test_reallocarray_strict_default( void)
{
   void   *p;

   p = mulle_reallocarray_strict( NULL, 4, sizeof( int));
   if( ! p)
      return( -1);

   // n == 0: frees block and returns NULL
   if( mulle_reallocarray_strict( p, 0, sizeof( int)))
      return( -2);

   p = mulle_reallocarray_strict( NULL, 4, sizeof( int));
   if( ! p)
      return( -3);

   // size == 0: frees block and returns NULL
   if( mulle_reallocarray_strict( p, 4, 0))
      return( -4);

   return( 0);
}


int  main( int argc, char *argv[])
{
   int  rc;

   rc = 0;
   rc |= test_reallocarray( NULL);
   rc |= test_reallocarray( &mulle_default_allocator);
   rc |= test_reallocarray( &mulle_stdlib_allocator);
   rc |= test_reallocarray( &mulle_stdlib_nofree_allocator);

   rc |= test_reallocarray_strict( NULL);
   rc |= test_reallocarray_strict( &mulle_default_allocator);
   rc |= test_reallocarray_strict( &mulle_stdlib_allocator);
   rc |= test_reallocarray_strict( &mulle_stdlib_nofree_allocator);

   // default allocator convenience wrappers
   rc |= test_reallocarray_default();
   rc |= test_reallocarray_strict_default();

   return( rc);
}