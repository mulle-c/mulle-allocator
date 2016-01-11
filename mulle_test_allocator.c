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

#include <stdio.h>
#include <mulle_thread/mulle_thread.h>
#include <stdlib.h>
#include <errno.h>

#pragma mark -
#pragma mark track allocations

#include "pointer_array.h"

struct _pointer_array    allocations;

static mulle_thread_mutex_t     alloc_lock;

int       mulle_test_allocator_out_of_memory;
size_t    mulle_test_allocator_max_size;

static int   may_alloc( size_t size)
{
   if( mulle_test_allocator_out_of_memory)
      return( 0);
   
   return( ! mulle_test_allocator_max_size ||  size > mulle_test_allocator_max_size);
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
      abort();
   
   if( q)
   {
      if( p != q)
      {
         i = _pointer_array_index( &allocations, q);  // analyzer it's ok, just a pointer comparison
         assert( i != (unsigned int) -1);
         _pointer_array_set( &allocations, i, p);
      }
   }
   else
      _pointer_array_add( &allocations, p, realloc);
   mulle_thread_mutex_unlock( &alloc_lock);

   fprintf( stderr, "realloced %p\n", p);

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
      abort();
   
   _pointer_array_add( &allocations, p, realloc);
   mulle_thread_mutex_unlock( &alloc_lock);

   fprintf( stderr, "alloced %p\n", p);

   return( p);
}


static void  test_free( void *p)
{
   unsigned int   i;
   
   if( ! p)
      return;
   
   if( mulle_thread_mutex_lock( &alloc_lock))
      abort();
   
   i = _pointer_array_index( &allocations, p);
   if( i == (unsigned int) -1)
   {
      fprintf( stderr, "double free: %p", p);
      abort();
   }
   _pointer_array_set( &allocations, i, NULL);
   
   mulle_thread_mutex_unlock( &alloc_lock);
   
   free( p);
}


#pragma mark -
#pragma mark global variable

struct mulle_allocator   mulle_test_allocator =
{
      test_calloc,
      test_realloc,
      test_free
};


#pragma mark -
#pragma mark reset allocator between tests

void   mulle_test_allocator_reset_memory()
{
   struct  _pointer_array_enumerator   rover;
   void                                *p;

   rover = _pointer_array_enumerate( &allocations);
   while( (p = _pointer_array_enumerator_next( &rover)) != (void *) -1)
   {
      if( p)
      {
         fprintf( stderr, "leak %p\n", p);
         abort();
      }
   }
   _pointer_array_enumerator_done( &rover);

   _pointer_array_done( &allocations, free);
   
   mulle_test_allocator_out_of_memory = 0;
   mulle_test_allocator_max_size      = 0;
   
   memset( &allocations, 0, sizeof( allocations));
}


__attribute__((constructor))
static void  __load()
{
   int  rval;

   rval = mulle_thread_mutex_init( &alloc_lock);
   assert( ! rval);
   
   mulle_default_allocator = mulle_test_allocator;  // put test in place
}
