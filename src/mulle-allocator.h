//  mulle-container
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
#ifndef mulle_allocator_h__
#define mulle_allocator_h__

#include "dependencies.h"

#include "mulle-allocator-struct.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#if MULLE_C11_VERSION < ((1 << 20) | (3 << 8) | 0)
# error "mulle_c11 is too old"
#endif

//
// community version is always even
//
#define MULLE_ALLOCATOR_VERSION  ((2 << 20) | (2 << 8) | 13)


#ifndef MULLE_ALLOCATOR_EXTERN_GLOBAL
# define MULLE_ALLOCATOR_EXTERN_GLOBAL    MULLE_C_EXTERN_GLOBAL
#endif


MULLE_ALLOCATOR_EXTERN_GLOBAL struct mulle_allocator   mulle_default_allocator;
MULLE_ALLOCATOR_EXTERN_GLOBAL struct mulle_allocator   mulle_stdlib_allocator;

MULLE_C_NO_RETURN
void   mulle_allocator_fail( void *block, size_t size);


# pragma mark -
# pragma mark Petty Accessors

static inline void   mulle_allocator_set_aba( struct mulle_allocator *p,
                                              void *aba,
                                              int (*f)( void *aba, int (*free)( void *), void *block))
{
   if( ! p)
      p = &mulle_default_allocator;

   p->aba     = aba;
   p->abafree = f ? (int (*)()) f : (int (*)()) abort;
}


static inline void   mulle_allocator_set_fail( struct mulle_allocator *p,
                                               void (*f)( void *block, size_t size) _MULLE_C_NO_RETURN)
{
   if( ! p)
      p = &mulle_default_allocator;

   p->fail = f ? f : mulle_allocator_fail;
}


# pragma mark -
# pragma mark Vectoring


MULLE_C_NON_NULL_RETURN
static inline void   *_mulle_allocator_malloc( struct mulle_allocator *p, size_t size)
{
   void   *q;

   q = (*p->realloc)( NULL, size);
   if( ! q)
      (*p->fail)( NULL, size);
   return( q);
}


MULLE_C_NON_NULL_RETURN
static inline void   *_mulle_allocator_calloc( struct mulle_allocator *p, size_t n, size_t size)
{
   void   *q;

   q = (*p->calloc)( n, size);
   if( ! q)
      (*p->fail)( NULL, n * size);
   return( q);
}


MULLE_C_NON_NULL_RETURN
static inline void   *_mulle_allocator_realloc( struct mulle_allocator *p, void *block, size_t size)
{
   void   *q;

   q = (*p->realloc)( block, size);
   if( ! q)
      (*p->fail)( block, size);
   return( q);
}


static inline void   _mulle_allocator_free( struct mulle_allocator *p, void *block)
{
   if( block)
      (*p->free)( block);
}


static inline int   _mulle_allocator_abafree( struct mulle_allocator *p, void *block)
{
   if( ! block)
      return( 0);
   return( (*p->abafree)( p->aba, p->free, block));
}


static inline void   mulle_allocator_assert( struct mulle_allocator *p)
{
   assert( (p ? p : &mulle_default_allocator));
   assert( (p ? p : &mulle_default_allocator)->calloc);
   assert( (p ? p : &mulle_default_allocator)->realloc);
   assert( (p ? p : &mulle_default_allocator)->free);
   assert( (p ? p : &mulle_default_allocator)->fail);
}


# pragma mark -
# pragma mark API

MULLE_C_NON_NULL_RETURN
static inline void   *mulle_allocator_malloc( struct mulle_allocator *p, size_t size)
{
   return( _mulle_allocator_malloc( p ? p : &mulle_default_allocator, size));
}


MULLE_C_NON_NULL_RETURN
static inline void   *mulle_allocator_calloc( struct mulle_allocator *p, size_t n, size_t size)
{
   return( _mulle_allocator_calloc( p ? p : &mulle_default_allocator, n, size));
}


MULLE_C_NON_NULL_RETURN
static inline void   *mulle_allocator_realloc( struct mulle_allocator *p, void *block, size_t size)
{
   return( _mulle_allocator_realloc( p ? p : &mulle_default_allocator, block, size));
}


static inline void   mulle_allocator_free( struct mulle_allocator *p, void *block)
{
   _mulle_allocator_free( p ? p : &mulle_default_allocator, block);
}


static inline int   mulle_allocator_abafree( struct mulle_allocator *p, void *block)
{
   return( _mulle_allocator_abafree( p ? p : &mulle_default_allocator, block));
}


# pragma mark -
# pragma mark Convenience API

MULLE_C_NON_NULL_RETURN
static inline void   *mulle_malloc( size_t size)
{
   return( _mulle_allocator_malloc( &mulle_default_allocator, size));
}


MULLE_C_NON_NULL_RETURN
static inline void   *mulle_calloc( size_t n, size_t size)
{
   return( _mulle_allocator_calloc( &mulle_default_allocator, n, size));
}


MULLE_C_NON_NULL_RETURN
static inline void   *mulle_realloc( void *block, size_t size)
{
   return( _mulle_allocator_realloc( &mulle_default_allocator, block, size));
}


static inline void   mulle_free( void *block)
{
   _mulle_allocator_free( &mulle_default_allocator, block);
}


static inline int   mulle_abafree( void *block)
{
   return( _mulle_allocator_abafree( &mulle_default_allocator, block));
}


# pragma mark -
# pragma mark strdup convenience

static inline char   *_mulle_allocator_strdup( struct mulle_allocator *p, char *s)
{
   size_t   len;
   char     *dup;

   len = strlen( s) + 1;
   dup = _mulle_allocator_malloc( p, len);
   if( dup)
      memcpy( dup, s, len);
   return( dup);
}


# pragma mark -
# pragma mark strdup API

static inline char   *mulle_allocator_strdup( struct mulle_allocator *p, char *s)
{
   if( ! s)
      return( s);
   return( _mulle_allocator_strdup( p ? p : &mulle_default_allocator, s));
}


static inline char   *mulle_strdup( char *s)
{
   return( mulle_allocator_strdup( &mulle_default_allocator, s));
}

#endif
