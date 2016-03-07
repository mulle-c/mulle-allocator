//
//  mulle_test_allocator.c
//  mulle-container
//
//  Created by Nat! on 04.11.15.
//  Copyright © 2015 Mulle kybernetiK. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//

#include "mulle_test_allocator.h"

#include <mulle_thread/mulle_thread.h>
#include "mulle_allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef __APPLE__
# include <execinfo.h>
# include <stdio.h>
#endif


#pragma mark -
#pragma mark track allocations

#include "pointerarray.h"

struct _pointerarray            allocations;
static mulle_thread_mutex_t     alloc_lock;

int       mulle_test_allocator_out_of_memory;
size_t    mulle_test_allocator_max_size;

static int  trace = -1;


static int   may_alloc( size_t size)
{
   if( mulle_test_allocator_out_of_memory)
      return( 0);
   
   return( ! mulle_test_allocator_max_size ||  size > mulle_test_allocator_max_size);
}


#ifdef __APPLE__
//
// it gives 0   libMulleStandaloneObjC.dylib        0x000000010ab7e46d stacktrace + 61
// move past the address
static char   *trim_belly_fat( char *s)
{
   char   *hex;
   char   *sym;
   
   hex = strstr( s, " 0x");
   if( ! hex)
      return( s);
   hex++;
   
   sym = strchr( hex, ' ');
   if( ! sym)
      return( hex);
   
   while( *sym == ' ')
      ++sym;
   if( ! *sym)
      return( hex);

   return( sym);
}


static int   trim_arse_fat( char *s)
{
   char   *offset;

   offset = strstr( s, " + ");
   if( ! offset)
      return( (int) strlen( s));
   
   return( (int) (offset - s));
}


static int  boring( char *s, int size)
{
   if( size == 3 && ! strncmp( s, "0x0", 3))
      return( 1);
   
   if( ! strncmp( s, "test_calloc_or_raise", strlen( "test_calloc_or_raise")))
      return( 1);
   if( ! strncmp( s, "test_realloc_or_raise", strlen( "test_realloc_or_raise")))
      return( 1);
   if( ! strncmp( s, "test_realloc", strlen( "test_realloc")))
      return( 1);
   if( ! strncmp( s, "test_calloc", strlen( "test_calloc")))
      return( 1);
   if( ! strncmp( s, "test_free", strlen( "test_free")))
      return( 1);

   return( 0);
}

#endif


static void   stacktrace( int offset)
{
   fprintf( stderr, " : [");
#ifdef __APPLE__
   {
      void   *callstack[256];
      int    frames;
      char   **strs;
      char   **p;
      char   **sentinel;
      char   *delim;
      char   *s;
      int    size;
      
      frames   = backtrace( callstack, 256);
      strs     = backtrace_symbols( callstack, frames);
      p        = &strs[ frames];
      sentinel = &strs[ offset];
      
      delim = "";
      while( p > sentinel)
      {
         s = *--p;
         if( trace > 2)
         {
            fprintf( stderr, "%s%s", delim, s);
            delim = "|";
         }
         else
         {
            s    = trim_belly_fat( s);
            size = trim_arse_fat( s);
            if( ! boring( s, size))
            {
               fprintf( stderr, "%s%.*s", delim, size, s);
               delim = "|";
            }
         }
      }
      free( strs);
   }
#endif
   fputc( ']', stderr);
}


static void   bail( void *p)
{
#ifdef __APPLE__
   {
      /* start malloc_history */
      char buf [ 256];
      extern int  getpid( void);
      
      sprintf( buf, "sudo malloc_history %d %p", getpid(), p);
      fprintf( stderr, "%s\n", buf);
      system( buf);
   }
#endif
   abort();
}



static void  *test_realloc( void *q, size_t size)
{
   void           *p;
   unsigned int   i;
   
   if( ! may_alloc( size))
   {
      errno = ENOMEM;
      return( NULL);
   }
   
   p = realloc( q, size);
   if( ! p)
      return( p);

   if( mulle_thread_mutex_lock( &alloc_lock))
   {
      perror( "mulle_thread_mutex_lock:");
      abort();
   }
   
   if( q)
   {
      if( p != q)
      {
         i = _pointerarray_index( &allocations, q);  // analyzer it's ok, just a pointer comparison
         assert( i != (unsigned int) -1);
         _pointerarray_set( &allocations, i, p);
      }
   }
   else
      _pointerarray_add( &allocations, p, realloc);
   mulle_thread_mutex_unlock( &alloc_lock);

   if( trace)
   {
      fprintf( stderr, "realloced %p-%p", p, &((char *)p)[ size ? size - 1 : 0]);
      if( trace > 1)
         stacktrace( 1);
      fputc( '\n', stderr);
   }
   return( p);
}


static void  *test_calloc( size_t n, size_t size)
{
   void   *p;
   
   if( ! may_alloc( size))
   {
      errno = ENOMEM;
      return( NULL);
   }

   p = calloc( n, size);
   if( ! p)
      return( p);
   
   if( mulle_thread_mutex_lock( &alloc_lock))
   {
      perror( "mulle_thread_mutex_lock:");
      abort();
   }
   
   _pointerarray_add( &allocations, p, realloc);
   mulle_thread_mutex_unlock( &alloc_lock);

   if( trace)
   {
      fprintf( stderr, "alloced %p-%p", p, &((char *)p)[ n * size ? n * size - 1 : 0]);
      if( trace > 1)
         stacktrace( 1);
      fputc( '\n', stderr);
   }

   return( p);
}


static void  test_free( void *p)
{
   unsigned int   i;
   
   if( ! p)
      return;
   
   if( mulle_thread_mutex_lock( &alloc_lock))
   {
      perror( "mulle_thread_mutex_lock:");
      abort();
   }
   
   i = _pointerarray_index( &allocations, p);
   if( i == (unsigned int) -1)
   {
      fprintf( stderr, "double free: %p", p);
      if( trace > 1)
         stacktrace( 1);
      fputc( '\n', stderr);

      bail( p);
   }
   _pointerarray_set( &allocations, i, NULL);
   
   mulle_thread_mutex_unlock( &alloc_lock);
   
   free( p);
   
   if( trace)
   {
      fprintf( stderr, "freed %p", p);  // analyzer: just an address print
      if( trace > 1)
         stacktrace( 1);
      fputc( '\n', stderr);
   }
}


#pragma mark -
#pragma mark global variable

struct mulle_allocator   mulle_test_allocator =
{
   test_calloc,
   test_realloc,
   test_free,
   (void *) abort,
   NULL
};


#pragma mark -
#pragma mark reset allocator between tests

void   mulle_test_allocator_set_tracelevel( unsigned int value)
{
   assert( value <= 3);
   if( (int) value != trace && (trace != -1 || value))
      fprintf( stderr, "mulle_test_allocator: trace level set to %d\n", value);

   trace = value;
}


void   mulle_test_allocator_reset()
{
   struct  _pointerarray_enumerator   rover;
   void                               *p;
   void                               *first_leak;
   
   if( mulle_thread_mutex_lock( &alloc_lock))
   {
      perror( "mulle_thread_mutex_lock:");
      abort();
   }

   first_leak = NULL;
   
   rover = _pointerarray_enumerate( &allocations);
   while( (p = _pointerarray_enumerator_next( &rover)) != (void *) -1)
   {
      if( p)
      {
         fprintf( stderr, "leak %p\n", p);
         if( ! first_leak)
            first_leak = p;
      }
   }
   _pointerarray_enumerator_done( &rover);

   if( first_leak)
      bail( first_leak);
   
   _pointerarray_done( &allocations, free);
   
   mulle_test_allocator_out_of_memory = 0;
   mulle_test_allocator_max_size      = 0;
   
   memset( &allocations, 0, sizeof( allocations));

   mulle_thread_mutex_unlock( &alloc_lock);
}


__attribute__((constructor))
void   mulle_test_allocator_initialize()
{
   int    rval;
   char   *s;

   if( trace != -1)
      return;
   
   rval = mulle_thread_mutex_init( &alloc_lock);
   assert( ! rval);
   
   mulle_default_allocator = mulle_test_allocator;  // put test in place
   
   s = getenv( "MULLE_TEST_ALLOCATOR_TRACE");
   mulle_test_allocator_set_tracelevel( s ? atoi( s) : 0);
}
