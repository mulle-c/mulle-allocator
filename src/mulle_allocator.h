//  mulle-container
//
//  Created by Nat! on 02/11/15.
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
#ifndef mulle_allocator_h__
#define mulle_allocator_h__


#include <stddef.h>


#define MULLE_ALLOCATOR_VERSION  ((1 << 20) | (0 << 8) | 0)

//
// mulle_allocator: a way to pass around the memory scheme du jour
//                  for transparent mulle_aba support, there is the
//                  smart option.
//
struct mulle_allocator
{
   union
   {
      struct
      {
         void   *(*calloc)( size_t, size_t);
         void   *(*realloc)( void *, size_t);
         void   (*free)( void *);
      } plain;
      struct
      {
         void   *(*calloc)( struct mulle_allocator *, size_t, size_t);
         void   *(*realloc)( struct mulle_allocator *, void *, size_t);
         void   (*free)( struct mulle_allocator *, void *);
      } smart;
   } vectors;
   unsigned int  mode;
};


extern struct mulle_allocator   mulle_default_allocator;
extern struct mulle_allocator   mulle_stdlib_allocator;


static inline void  *_mulle_allocator_malloc( struct mulle_allocator *p, size_t size)
{
   if( p->mode)
      return( (*p->vectors.smart.realloc)( p, NULL, size));
   return( (*p->vectors.plain.realloc)( NULL, size));
}


static inline void  *_mulle_allocator_calloc( struct mulle_allocator *p, size_t n, size_t size)
{
   if( p->mode)
      return( (*p->vectors.smart.calloc)( p, n, size));
   return( (*p->vectors.plain.calloc)( n, size));
}


static inline void  *_mulle_allocator_realloc( struct mulle_allocator *p, void *block, size_t size)
{
   if( p->mode)
      return( (*p->vectors.smart.realloc)( p, block, size));
   return( (*p->vectors.plain.realloc)( block, size));
}


static inline void  _mulle_allocator_free( struct mulle_allocator *p, void *block)
{
   if( p->mode)
      return( (*p->vectors.smart.free)( p, block));
   (*p->vectors.plain.free)( block);
}


# pragma mark -
# pragma mark API

static inline void  *mulle_allocator_malloc( struct mulle_allocator *p, size_t size)
{
   return( _mulle_allocator_malloc( p ? p : &mulle_default_allocator, size));
}


static inline void  *mulle_allocator_calloc( struct mulle_allocator *p, size_t n, size_t size)
{
   return( _mulle_allocator_calloc( p ? p : &mulle_default_allocator, n, size));
}


static inline void  *mulle_allocator_realloc( struct mulle_allocator *p, void *block, size_t size)
{
   return( _mulle_allocator_realloc( p ? p : &mulle_default_allocator, block, size));
}


static inline void  mulle_allocator_free( struct mulle_allocator *p, void *block)
{
   return( _mulle_allocator_free( p ? p : &mulle_default_allocator, block));
}


# pragma mark -
# pragma mark Convenience

static inline void  *mulle_malloc( size_t size)
{
   return( _mulle_allocator_malloc( &mulle_default_allocator, size));
}


static inline void  *mulle_calloc( size_t n, size_t size)
{
   return( _mulle_allocator_calloc( &mulle_default_allocator, n, size));
}


static inline void  *mulle_realloc( void *block, size_t size)
{
   return( _mulle_allocator_realloc( &mulle_default_allocator, block, size));
}


static inline void  mulle_free( void *block)
{
   return( _mulle_allocator_free( &mulle_default_allocator, block));
}

#endif
