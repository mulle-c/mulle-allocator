#include <mulle-allocator/mulle-allocator.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>


// Coverage for macros that were never instantiated anywhere, so they were
// never compiled:
//
//    mulle_malloc_do
//    mulle_calloc_do
//    mulle_alloca_do_flexible
//    mulle_calloca_do_flexible
//    _mulle_calloca_do_return_void
//    mulle_flexarray_do, mulle_flexarray_realloc
//    _mulle_flexarray_return, _mulle_flexarray_return_void
//
// The stacksize/stackcount arguments have to be constant expressions, as they
// size the stack resident array. The `count` argument can be a runtime value.
//


static int   did_return_void;


static int   test_malloc_do( unsigned int amount)
{
   unsigned int  i;

   mulle_malloc_do( values, int, amount)
   {
      for( i = 0; i < amount; i++)
         values[ i] = (int) i;
      for( i = 0; i < amount; i++)
         if( values[ i] != (int) i)
            return( 1);
   }
   return( 0);
}


static int   test_calloc_do( unsigned int amount)
{
   unsigned int  i;

   mulle_calloc_do( values, int, amount)
   {
      for( i = 0; i < amount; i++)
         if( values[ i] != 0)
            return( 1);
      for( i = 0; i < amount; i++)
         values[ i] = (int) i;
      for( i = 0; i < amount; i++)
         if( values[ i] != (int) i)
            return( 2);
   }
   return( 0);
}


// 128 bytes of stack are 32 ints, 16 bytes are 4 ints
static int   test_alloca_do_flexible( unsigned int count)
{
   unsigned int  i;

   mulle_alloca_do_flexible( values, int, 128, count)
   {
      if( (count <= 32) != (values == values__storage))
         return( 1);

      for( i = 0; i < count; i++)
         values[ i] = (int) i;
      for( i = 0; i < count; i++)
         if( values[ i] != (int) i)
            return( 2);
   }

   mulle_alloca_do_flexible( values, int, 16, count)
   {
      if( (count <= 4) != (values == values__storage))
         return( 3);

      for( i = 0; i < count; i++)
         values[ i] = (int) i;
      for( i = 0; i < count; i++)
         if( values[ i] != (int) i)
            return( 4);
   }
   return( 0);
}


static int   test_calloca_do_flexible( unsigned int count)
{
   unsigned int  i;

   mulle_calloca_do_flexible( values, int, 128, count)
   {
      if( (count <= 32) != (values == values__storage))
         return( 1);

      for( i = 0; i < count; i++)
         if( values[ i] != 0)
            return( 2);
      for( i = 0; i < count; i++)
         values[ i] = (int) i;
      for( i = 0; i < count; i++)
         if( values[ i] != (int) i)
            return( 3);
   }

   mulle_calloca_do_flexible( values, int, 16, count)
   {
      if( (count <= 4) != (values == values__storage))
         return( 4);

      for( i = 0; i < count; i++)
         if( values[ i] != 0)
            return( 5);
      for( i = 0; i < count; i++)
         values[ i] = (int) i;
      for( i = 0; i < count; i++)
         if( values[ i] != (int) i)
            return( 6);
   }
   return( 0);
}


static void  test_calloca_do_return_void( unsigned int amount)
{
   mulle_calloca_do( values, int, amount)
   {
      did_return_void = 1;
      _mulle_calloca_do_return_void( values);
   }
   did_return_void = 2;
}


// stackcount is the number of `type` elements kept on the stack
static int   test_flexarray_do( unsigned int count)
{
   unsigned int  i;

   mulle_flexarray_do( values, int, 32, count)
   {
      if( (count <= 32) != (values == values__storage))
         return( 1);

      for( i = 0; i < count; i++)
         values[ i] = (int) i;

      mulle_flexarray_realloc( values, count * 2);
      for( i = 0; i < count; i++)
         if( values[ i] != (int) i)
            return( 2);
   }

   mulle_flexarray_do( values, int, 1, count)
   {
      if( (count <= 1) != (values == values__storage))
         return( 3);

      for( i = 0; i < count; i++)
         values[ i] = (int) i;

      mulle_flexarray_realloc( values, count * 2);
      for( i = 0; i < count; i++)
         if( values[ i] != (int) i)
            return( 4);
   }
   return( 0);
}


#if MULLE_C_HAS_TYPEOF
static double  test_flexarray_return( unsigned int amount)
{
   mulle_flexarray_do( values, int, 8, amount)
   {
      _mulle_flexarray_return( values, (double) amount);
   }
   return( 0.0);
}
#endif


static void  test_flexarray_return_void( unsigned int amount)
{
   mulle_flexarray_do( values, int, 8, amount)
   {
      did_return_void = 1;
      _mulle_flexarray_return_void( values);
   }
   did_return_void = 2;
}


int  main( void)
{
   int  rc;

   rc = 0;

   rc |= test_malloc_do( 1);
   rc |= test_malloc_do( 64);

   rc |= test_calloc_do( 1);
   rc |= test_calloc_do( 64);

   rc |= test_alloca_do_flexible( 32);     // stack
   rc |= test_alloca_do_flexible( 33);     // heap
   rc |= test_alloca_do_flexible( 4);      // stack
   rc |= test_alloca_do_flexible( 5);      // heap

   rc |= test_calloca_do_flexible( 32);    // stack
   rc |= test_calloca_do_flexible( 33);    // heap
   rc |= test_calloca_do_flexible( 4);     // stack
   rc |= test_calloca_do_flexible( 5);     // heap

   did_return_void = 0;
   test_calloca_do_return_void( 4);
   rc |= did_return_void != 1;

   rc |= test_flexarray_do( 5);            // stack
   rc |= test_flexarray_do( 32);           // stack, grows to heap
   rc |= test_flexarray_do( 33);           // heap

#if MULLE_C_HAS_TYPEOF
   if( test_flexarray_return( 1000) != 1000.0)
      rc |= 1;
#endif

   did_return_void = 0;
   test_flexarray_return_void( 4);
   rc |= did_return_void != 1;

   return( rc != 0);
}
