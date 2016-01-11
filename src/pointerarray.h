//
//  pointerarray.h
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
#ifndef pointerarray_h__
#define pointerarray_h__

#pragma mark -
#pragma mark _pointerarray, simple growing array all inlined used for testing

#include <string.h>


struct _pointerarray
{
   size_t   count;
   size_t   used;
   size_t   size;
   void     **pointers;
};


static inline struct _pointerarray  *_pointerarray_alloc( void *(*calloc)( size_t, size_t))
{
   return( (struct _pointerarray *) (*calloc)( 1, sizeof( struct _pointerarray)));
}


static int   _pointerarray_grow( struct _pointerarray *array, void *(*realloc)( void *, size_t))
{
   size_t   new_size;

   new_size = array->size * 2;
   if( new_size < 2)
      new_size = 2;
   
   array->pointers = (*realloc)( array->pointers, sizeof( void *) * new_size);
   if( ! array->pointers)
   {
      array->size = 0;
      assert( 0);
      return( -1);
   }

   memset( &array->pointers[ array->size], 0, sizeof( void *) * (new_size - array->size));
   array->size = new_size;

   return( 0);
}


static inline int   _pointerarray_add( struct _pointerarray *array, void  *pointer,
                                       void *(*realloc)( void *, size_t))
{
   if( array->used == array->size)
      if( _pointerarray_grow( array, realloc))
      {
         assert( 0);
         return( -1);
      }
   
   array->pointers[ array->used++] = pointer;
   array->count++;
   
   return( 0);
}


static inline void  *_pointerarray_get( struct _pointerarray *array, unsigned int i)
{
   assert( array);
   assert( i < array->used);
   return( array->pointers[ i]);
}


static inline unsigned int   _pointerarray_index( struct _pointerarray *array, void *p)
{
   void       **curr;
   void       **sentinel;

   curr     = array->pointers;
   sentinel = &curr[ array->used];
   while( curr < sentinel)
   {
      if( *curr == p)
         return( (unsigned int) (curr - array->pointers));
      curr++;
   }
   return( (unsigned int) -1);
}


static inline void   _pointerarray_set( struct _pointerarray *array, unsigned int i, void *p)
{
   void   *old;

   assert( array);
   assert( i < array->used);
   
   old = array->pointers[ i];
   if( p)
      array->count += old == NULL;
   else
      array->count -= old != NULL;
   
   array->pointers[ i] = p;
}


static inline size_t  _pointerarray_count_non_null_pointers( struct _pointerarray *array)
{
   return( array->count);
}


static inline void  _pointerarray_done( struct _pointerarray *array, void (*free)( void *))
{
   (*free)( array->pointers);
}


static inline void  array_free( struct _pointerarray *array, void (*free)( void *))
{
   _pointerarray_done( array, free);
   (*free)( array);
}


struct _pointerarray_enumerator
{
   void   **curr;
   void   **sentinel;
};


static inline struct  _pointerarray_enumerator   _pointerarray_enumerate( struct _pointerarray *array)
{
   struct _pointerarray_enumerator   rover;
   
   rover.curr     = &array->pointers[ 0];
   rover.sentinel = &rover.curr[ array->used];
   
   assert( rover.sentinel >= rover.curr);
   
   return( rover);
}


static inline void   *_pointerarray_enumerator_next( struct _pointerarray_enumerator *rover)
{
   return( rover->curr < rover->sentinel ? *rover->curr++ : (void *) -1);
}


static inline void  _pointerarray_enumerator_done( struct _pointerarray_enumerator *rover)
{
}

#endif
