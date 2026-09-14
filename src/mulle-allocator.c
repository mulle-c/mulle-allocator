//
//  mulle-allocator.c
//  mulle-allocator
//
//  Copyright (c) 2018 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
//  All rights reserved.
//
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
#include "mulle-allocator.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


MULLE_C_NO_RETURN
void   mulle_allocation_fail( struct mulle_allocator *p,
                              void *block,
                              size_t size)
{
   MULLE_C_UNUSED( p);
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( size);

   fprintf( stderr, "mulle-allocator: memory allocation of %zu bytes failed\n", size);
   abort();
}


MULLE_C_NONNULL_RETURN
char   *_mulle_allocator_strdup( struct mulle_allocator *p, const char *s)
{
   size_t   size;
   char     *dup;

   // NULL check is in mulle_allocator_strdup(s)
   // this crashes by design

   size = strlen( s) + 1;
   dup  = (*p->reallocarray)( NULL, 1, size, p);
   if( dup)
      memcpy( dup, s, size);
   else
      // fail must not return
      (*p->fail)( p, NULL, size);
   return( dup);
}



void   *_mulle_allocator_realloc_strict( struct mulle_allocator *p,
                                         void *block,
                                         size_t size)
{
   void   *q;

   if( ! size)
   {
      if( block)
         (*p->free)( block, p);
      return( NULL);
   }

   q = (*p->reallocarray)( block, 1, size, p);
   if( ! q)
      (*p->fail)( p, block, size);
   return( q);
}


void *  _mulle_allocator_reallocarray_strict( struct mulle_allocator *p,
                                              void *block,
                                              size_t n,
                                              size_t size)
{
   void   *q;

   // stdlib-like: free and return NULL, instead of being a programmer error
   if( ! n || ! size)
   {
      if( block)
         (*p->free)( block, p);
      return( NULL);
   }

   q = (*p->reallocarray)( block, n, size, p);
   if( MULLE_C_UNLIKELY( ! q))
      (*p->fail)( p, block, mulle_allocator_size_multiply( p, n, size));
   return( q);
}



//
// The default abafree reclaimer: no ABA-safe freeing is configured, which is
// a programmer error. Returns int to match the abafree vector, but never
// returns: it aborts.
//
MULLE_C_NO_RETURN
int   mulle_allocator_no_aba_abort( void *aba,
                                    void (*free)( void *, void *),
                                    void *block,
                                    void *owner)
{
   MULLE_C_UNUSED( aba);
   MULLE_C_UNUSED( free);
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( owner);

   abort();
}


//
// The allocator's deferred-free adapter. An ABA reclaimer calls this with the
// owner it was given (always the owning allocator); the block is released
// through that allocator's own `free`. The type is exactly
// mulle_allocator_deferred_free_t, so no function pointer is ever cast.
//
void   _mulle_allocator_deferred_free( void *block, void *owner)
{
   struct mulle_allocator   *p;

   p = owner;
   (*p->free)( block, p);
}


//
// The default reallocarray vector. reallocarray(3) is not standard C, so this
// is built on standard realloc. That means we perform the count*size
// multiplication ourselves and therefore we MUST check it. A zero count/size
// is likewise our programmer error. Both are routed through the allocator's
// `fail` vector. It backs malloc, realloc, allocarray and reallocarray alike.
//
static void   *v_reallocarray( void *block,
                               size_t n,
                               size_t size,
                               struct mulle_allocator *allocator)
{
   if( MULLE_C_UNLIKELY( ! n || ! size))
      _mulle_allocator_abort( allocator, block, 0);

   return( realloc( block, mulle_allocator_size_multiply( allocator, n, size)));
}


// #pragma mark - _mulle_allocator

//
// Typed abort stubs for _mulle_allocator_invalidate. Using real functions with
// the vector signatures avoids casting `abort` to an incompatible function
// pointer type.
//
MULLE_C_NO_RETURN
static void   *v_abort_calloc( size_t n,
                               size_t size,
                               struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( n);
   MULLE_C_UNUSED( size);
   MULLE_C_UNUSED( allocator);
   abort();
}


MULLE_C_NO_RETURN
static void   *v_abort_reallocarray( void *block,
                                     size_t n,
                                     size_t size,
                                     struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( n);
   MULLE_C_UNUSED( size);
   MULLE_C_UNUSED( allocator);
   abort();
}


MULLE_C_NO_RETURN
static void   v_abort_free( void *block, struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( allocator);
   abort();
}


MULLE_C_NO_RETURN
static void   v_abort_fail( struct mulle_allocator *allocator,
                            void *block,
                            size_t size)
{
   MULLE_C_UNUSED( allocator);
   MULLE_C_UNUSED( block);
   MULLE_C_UNUSED( size);
   abort();
}


void   _mulle_allocator_invalidate( struct mulle_allocator *p)
{
   p->calloc       = v_abort_calloc;
   p->reallocarray = v_abort_reallocarray;
   p->free         = v_abort_free;
   p->fail         = v_abort_fail;
   p->abafree      = mulle_allocator_no_aba_abort;
   p->aba          = NULL;
}


static void   *v_calloc( size_t n,
                         size_t size,
                         struct mulle_allocator *allocator)
{
   // zero count/size is always our programmer error
   if( MULLE_C_UNLIKELY( ! n || ! size))
      _mulle_allocator_abort( allocator, NULL, 0);

#if ! MULLE_ALLOCATOR_TRUST_STDLIB
   // calloc does the count*size multiplication itself; only check it when we
   // do not trust the platform to have done so
   if( MULLE_C_UNLIKELY( size > SIZE_MAX / n))
      _mulle_allocator_abort( allocator, NULL, (size_t) -1);
#endif

   return( calloc( n, size));
}


static void   v_free( void *block, struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( allocator);

   free( block);
}


static void   v_no_free( void *ignored, struct mulle_allocator *allocator)
{
   MULLE_C_UNUSED( ignored);
   MULLE_C_UNUSED( allocator);
}


struct mulle_allocator   mulle_allocator_stdlib =
{
   v_calloc,
   v_reallocarray,
   v_free,
   mulle_allocation_fail,
   mulle_allocator_no_aba_abort,
   NULL
};


// this will be patched by the foundation
struct mulle_allocator   mulle_allocator_default =
{
   v_calloc,
   v_reallocarray,
   v_free,
   mulle_allocation_fail,
   mulle_allocator_no_aba_abort,
   NULL
};


struct mulle_allocator   mulle_allocator_stdlib_nofree =
{
   v_calloc,
   v_reallocarray,
   v_no_free,
   mulle_allocation_fail,
   mulle_allocator_no_aba_abort,
   NULL
};


int   mulle_allocator_is_stdlib_allocator( const struct mulle_allocator *p)
{
   if( ! p)
      p = &mulle_allocator_default;
   return( p->calloc == v_calloc);
}
