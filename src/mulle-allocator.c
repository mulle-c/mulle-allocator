//  mulle-allocator
//
//  Created by Nat! on 02/11/15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
//  All rights reserved.
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
#include "mulle-allocator-struct.h"
// #include "mulle_allocator.h"  // don't include for windows
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


MULLE_C_NO_RETURN
void   mulle_allocation_fail( struct mulle_allocator *p, void *block, size_t size)
{
   perror( "memory allocation:");
   abort();
}


MULLE_C_NONNULL_RETURN
char   *_mulle_allocator_strdup( struct mulle_allocator *p, char *s)
{
   size_t   size;
   char     *dup;

   size = strlen( s) + 1;
   dup  = (*p->realloc)( NULL, size, p);
   if( ! dup)
      (*p->fail)( p, NULL, size);

   memcpy( dup, s, size);
   return( dup);
}



void *
   _mulle_allocator_realloc_strict( struct mulle_allocator *p,
                                    void *block,
                                    size_t size)
{
   void   *q;

   if( ! size)
   {
      (*p->free)( block, p);
      return( NULL);
   }

   q = (*p->realloc)( block, size, p);
   if( ! q)
      (*p->fail)( p, block, size);
   return( q);
}


//
// this function exists because gcc is paining me. But this is also "wrong"
// because it's a supposedly int returning function that should be marked as
// "no-return". It's a classical type tragedy.
//
// MULLE_C_NO_RETURN
int   mulle_aba_abort( void *aba, void (*free)( void *, void *), void *block, void *owner)
{
   abort();
}


#pragma mark - _mulle_allocator

void   _mulle_allocator_invalidate( struct mulle_allocator *p)
{
   p->calloc  = (void *(*)( size_t, size_t, struct mulle_allocator *)) abort;
   p->realloc = (void *(*)( void *, size_t, struct mulle_allocator *)) abort;
   p->free    = (void (*)( void *, struct mulle_allocator *)) abort;
   p->abafree = mulle_aba_abort;
}


#ifdef NO_STDLIB_TRAMPOLINE_NEEDED

static void   no_free( void *ignored)
{
}


struct mulle_allocator   mulle_stdlib_allocator =
{
   calloc,
   realloc,
   free,
   mulle_allocation_fail,
   mulle_aba_abort,
   NULL
};


// this will be patched by the foundation
struct mulle_allocator   mulle_default_allocator =
{
   calloc,
   realloc,
   free,
   mulle_allocation_fail,
   mulle_aba_abort,
   NULL
};


struct mulle_allocator   mulle_stdlib_nofree_allocator =
{
   calloc,
   realloc,
   no_free,
   mulle_allocation_fail,
   mulle_aba_abort,
   NULL
};


int   mulle_allocator_is_stdlib_allocator( struct mulle_allocator *p)
{
   return( p->calloc == calloc);
}

#else

static void   *v_calloc( size_t n, size_t size, struct mulle_allocator *allocator)
{
   return( calloc( n, size));
}


static void   *v_realloc( void *block, size_t size, struct mulle_allocator *allocator)
{
   return( realloc( block, size));
}


static void   v_free( void *block, struct mulle_allocator *allocator)
{
   free( block);
}


static void   v_no_free( void *ignored, struct mulle_allocator *allocator)
{
}


struct mulle_allocator   mulle_stdlib_allocator =
{
   v_calloc,
   v_realloc,
   v_free,
   mulle_allocation_fail,
   mulle_aba_abort,
   NULL
};


// this will be patched by the foundation
struct mulle_allocator   mulle_default_allocator =
{
   v_calloc,
   v_realloc,
   v_free,
   mulle_allocation_fail,
   mulle_aba_abort,
   NULL
};


struct mulle_allocator   mulle_stdlib_nofree_allocator =
{
   v_calloc,
   v_realloc,
   v_no_free,
   mulle_allocation_fail,
   mulle_aba_abort,
   NULL
};


int   mulle_allocator_is_stdlib_allocator( struct mulle_allocator *p)
{
   return( p->calloc == v_calloc);
}

#endif
