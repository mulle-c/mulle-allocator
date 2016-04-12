//
//  pointerset.h
//  mulle-aba-test
//
//  Created by Nat! on 10.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
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
#ifndef pointerset_h__
#define pointerset_h__

#pragma mark -
#pragma mark _pointerset, simple growing hashtable all inlined used for testing

#include <string.h>


// can not store null pointers, can not store (void *) -1

struct _pointerset
{
   size_t   count;
   size_t   used;
   size_t   max;
   size_t   mask;
   void     **pointers;
};


static inline struct _pointerset  *_pointerset_create( void *(*calloc)( size_t, size_t))
{
   return( (struct _pointerset *) (*calloc)( 1, sizeof( struct _pointerset)));
}


static inline void  _pointerset_init( struct _pointerset *set)
{
   memset( set, 0, sizeof( struct _pointerset));
}


static inline void  _pointerset_done( struct _pointerset *set, void (*free)( void *))
{
   (*free)( set->pointers);
}


static inline void  _pointerset_free( struct _pointerset *set, void (*free)( void *))
{
   _pointerset_done( set, free);
   (*free)( set);
}

#pragma mark -
#pragma mark hash

                              
// from code.google.com/p/smhasher/wiki/MurmurHash3
static inline uint32_t  avalanche32( uint32_t h)
{
   h ^= h >> 16;
   h *= 0x85ebca6b;
   h ^= h >> 13;
   h *= 0xc2b2ae35;
   h ^= h >> 16;
   return h;
}
                              
                              
static inline uint64_t   avalanche64(uint64_t h)
{
   h ^= h >> 33;
   h *= 0xff51afd7ed558ccd;
   h ^= h >> 33;
   h *= 0xc4ceb9fe1a85ec53;
   h ^= h >> 33;
   return h;
}
                              
                              
static inline uintptr_t   avalanche( uintptr_t h)
{
   if( sizeof( uintptr_t) == sizeof( uint64_t))
      return( avalanche64( h));
   return( avalanche32( h));
}
                             
                              

static inline uintptr_t  _pointerset_hash( struct _pointerset *set, void *p)
{
   return( avalanche( (uintptr_t) p));
}


#pragma mark -
#pragma mark enumerator

struct _pointerset_enumerator
{
   void   **curr;
   void   **sentinel;
};


static inline struct  _pointerset_enumerator   _pointerset_enumerate( struct _pointerset *set)
{
   struct _pointerset_enumerator   rover;
   
   rover.curr     = &set->pointers[ 0];
   rover.sentinel = set->pointers ? &rover.curr[ set->mask + 1] : rover.curr;
   
   assert( rover.sentinel >= rover.curr);
   
   return( rover);
}


static inline void   *_pointerset_enumerator_next( struct _pointerset_enumerator *rover)
{
   void   *p;
   
   for(;;)
   {
      if( rover->curr >= rover->sentinel)
         return( NULL);

      p = *rover->curr++;
      if( p && p != (void *) -1)
         return( p);
   }
}


static inline void  _pointerset_enumerator_done( struct _pointerset_enumerator *rover)
{
}


#pragma mark -
#pragma mark put

static inline void   *_pointerset_sureadd( struct _pointerset *set,
                                           void  *pointer)
{
   uintptr_t      hash;
   unsigned int   i;
   void           *q;
   
   assert( set->count <= set->max);
   assert( set->used < set->max);
   assert( set->max < set->mask + 1);
   
   hash = _pointerset_hash( set, pointer);
   i    = (unsigned int) hash;
   
   for(;;)
   {
      i &= set->mask;
      q  = set->pointers[ i];
      if( ! q)
      {
         set->pointers[ i] = pointer;
         set->count++;
         set->used++;
         return( pointer);
      }

      if( q == (void *) -1)
      {
         set->pointers[ i] = pointer;
         set->count++;
         return( pointer);
      }
   
      if( q == pointer)
         return( NULL);
      
      ++i;
   }
}


static int   _pointerset_grow( struct _pointerset *set,
                               void *(*calloc)( size_t, size_t),
                               void (*free)( void *))
{
   size_t                          size;
   size_t                          copy_size;
   struct _pointerset_enumerator   rover;
   struct _pointerset              copy;
   void                            *p;
   
   size      = set->mask + 1;
   copy_size = size;
   
   // if set is only half used, don't double
   if( set->count * 2 > set->max)
      copy_size = size * 2;
   
   if( copy_size < 2)
   {
      copy_size = 2;
      size      = 1;
   }
   
   copy.count = 0;
   copy.used  = 0;
   copy.max   = copy_size - (copy_size >> 1);
   copy.mask  = copy_size - 1;
   if( copy.max > copy.mask)
      copy.max = copy_size;
   
   copy.pointers = (*calloc)( sizeof( void *), copy_size);
   if( ! copy.pointers)
   {
      set->mask     = 0;
      set->pointers = NULL;
      assert( 0);
      
      return( -1);
   }
   
   rover =_pointerset_enumerate( set);
   while( p = _pointerset_enumerator_next( &rover))
      _pointerset_sureadd( &copy, p);
   _pointerset_enumerator_done( &rover);
   
   (*free)( set->pointers);
   
   *set = copy;
   
   assert( set->count <= set->max);
   assert( set->used < set->max);
   assert( set->max < set->mask + 1);
   
   return( 0);
}
                              

static inline void   *_pointerset_add( struct _pointerset *set,
                                       void  *pointer,
                                       void *(*calloc)( size_t, size_t),
                                       void (*free)( void *))
{
   assert( set);
   assert( pointer);

   if( set->used >= set->max)
      if( _pointerset_grow( set, calloc, free))
      {
         assert( 0);
         return( (void *) -1);
      }
   
   return( _pointerset_sureadd( set, pointer));
}


#pragma mark -
#pragma mark get

static inline void  *_pointerset_get( struct _pointerset *set, void *pointer)
{
   uintptr_t      hash;
   unsigned int   i;
   void           *q;
   
   assert( set);
   assert( pointer);
   assert( pointer != (void *) -1);
   
   hash = _pointerset_hash( set, pointer);
   i    = (unsigned int) hash;

   if( ! set->pointers)
      return( NULL);
   
   for(;;)
   {
      i &= set->mask;
      q  = set->pointers[ i];
      if( ! q || q == pointer)
         return( q);
      
      ++i;
   }
}


static inline void  _pointerset_remove( struct _pointerset *set, void *pointer)
{
   uintptr_t      hash;
   unsigned int   i;
   void           *q;
   
   assert( set);
   assert( pointer);
   assert( pointer != (void *) -1);
   
   hash = _pointerset_hash( set, pointer);
   i    = (unsigned int) hash;
   
   if( ! set->pointers)
      return;
   
   for(;;)
   {
      i &= set->mask;
      q  = set->pointers[ i];
      if( ! q)
         return;
   
      if( q == pointer)
      {
         assert( set->count);
         
         set->pointers[ i] = (void *) -1;
         set->count--;
         return;
      }
      
      ++i;
   }
}

#endif
