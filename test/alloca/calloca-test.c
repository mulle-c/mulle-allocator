#include <mulle-allocator/mulle-allocator.h>

#include <string.h>
#include <stdio.h>


static int  test( unsigned int amount)
{
   unsigned int  i;
   unsigned int  j;
   unsigned int  old_amount;

   mulle_calloca_do( values, int, amount)
   {
      for( j = 0; j < amount; j++)
         if( values[ j] != 0)
            return( 1);

      for( i = 0; i < amount; i++)
         values[ i] = i;

      old_amount = amount;
      amount    *= 4;
      mulle_calloca_do_realloc( values, amount);
      for( j = old_amount; j < amount; j++)
         if( values[ j] != 0)
            return( 1);

      for( ; i < amount; i++)
         values[ i] = i;
      amount /= 2;

      mulle_calloca_do_realloc( values, amount);
      for( i = 0; i < amount; i++)
         if( values[ i] != i)
            return( 1);
   }
   return( 0);
}


static double  test_return( unsigned int amount)
{
   mulle_calloca_do( values, int, amount)
   {
      // do something
      _mulle_calloca_do_return( values, (double) amount);
   }
   return( 0.0);
}


static int  *_test_extract( unsigned int amount)
{
   int  *p;
   int  *rval;

   rval = NULL;
   mulle_calloca_do( values, int, amount)
   {
      mulle_calloca_do_for( values, p)
      {
         *p++ = amount;
      }

      mulle_calloca_do_extract( values, rval);
   }
   return( rval);
}


static int  test_extract( unsigned int amount)
{
   int  *buf;
   int  *p;

   buf = _test_extract( amount);

   mulle_calloc_for( buf, amount, p)
   {
      if( *p++ != amount)
         return( -1);
   }

   mulle_free( buf);
   return( 0);
}


//  MEMO: leaks should be caught by valgrind
int  main( int argc, char *argv[])
{
   int      rc;
   int      i;
   double   v;

   rc = 0;
   for( i = 0; i < 256; i = i ? i * 2 : 1)
      rc |= test( i);

   v   = test_return( 1000);
   rc |= v != 1000.0;

   rc  |= test_extract( 8);
   rc  |= test_extract( 1848);

   return( rc);
}