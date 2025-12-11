//
//  mulle-alloca.c
//  mulle-allocator
//
//  Copyright (c) 2023 Nat! - Mulle kybernetiK.
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
#ifndef mulle_alloca_h__
#define mulle_alloca_h__

#ifndef mulle_allocator_h__
# include "mulle-allocator.h"
#endif

#include <stdint.h>

//
// ```
// /* buffer not yet in scope */
//
// mulle_alloca_do( buffer, char, size)
// {
//    /* code block, where buffer is valid */
// }
//
// /* buffer no longer in scope (and freed) */
// ```
//
// For small amounts of storage `mulle_alloca_do` uses a stack based (`auto`)
// buffer, but will escalate to `mulle_malloc` for heavier workloads. An
// allocated buffer will be freed when the `mulle_alloca_do` code block exits.
// You can safely `break`from such a block, but in order to `return` without
// leaking you'd have to use the special `mulle_alloca_do_return` macro.
// (It's easier to postpone the return to the outside of the block).
//
// MEMO: __attribute__((warn_return_exits_block)) would be nice.
//
// You can `mulle_realloc` the buffer with `mulle_alloca_do_realloc`. In order
// to transfer the buffer outside of the `mulle_alloca_do` block scope, use
// `mulle_alloca_do_extract` with a variable defined outside of the block.
//
//
// Check out this "conventional" code:
//
// void  print_uppercase( char *s)
// {
//    char     tmp[ 32];
//    char     *copy = tmp;
//    size_t   i;
//    size_t   len;
//
//    len = strlen( s) + 1;
//    if( len > 32)
//       copy = malloc( len);
//
//    for( i = 0; i < len; i++)
//      copy[ i] = toupper( s[ i]);
//
//    printf( "%s\n", copy);
//
//    if( copy != copy_data)
//       free( copy);
// }
//
// transformed to mulle-allocator code:
//
// void  print_uppercase( char *s)
// {
//    size_t   len;
//    char     *t;
//
//    len = strlen( s) + 1;
//    mulle_alloca_do( copy, char, len)
//    {
//       mulle_alloca_do_for( copy, t) // one more nicety
//          *t++ = toupper( *s++);
//       printf( "%s\n", copy);
//    }
// }
//
#ifndef MULLE_ALLOCA_STACKSIZE
# define MULLE_ALLOCA_STACKSIZE  128 // bytes, equivalent of double[ 16]
#endif

/* helper routines not part of API */

#define _mulle_alloca_stackitems( size, type)                     \
   (size / sizeof( type))


//
// MEMO: could check if count exceeds desired size and then reduce to 1
//       to lower stack pressure, but does it make sense ?
//
#define _mulle_alloca_stackitems_1( size, type)                   \
   (_mulle_alloca_stackitems( size, type)                         \
    ? _mulle_alloca_stackitems( size, type)                       \
    : 1)

#define _mulle_alloca_do_get_size_as_length( name)                \
   (sizeof( name ## __storage) / sizeof( name ## __storage[ 0]))


// returns 1 for loop convenience
#define _mulle_alloca_do_free( name)                              \
  ((name != name ## __storage)                                    \
   ? (mulle_free( name), (void *) 1)                              \
   : (void *) 1)                                                  \


//
// These two macros are only useful to return from a single `mulle_alloca_do`.
// They won't magically do the right thing in nested mulle_alloca_dos.
// So use with caution, better: Don't use! `break` instead and
// `return` outside of `mulle_alloca_do`. Therefore not part of API.
//
#define _mulle_alloca_do_return( name, value)                     \
do                                                                \
{                                                                 \
   __typeof__( value) name ## __tmp = (value);                    \
                                                                  \
   (void) _mulle_alloca_do_free( name);                           \
   return( name ## __tmp);                                        \
}                                                                 \
while( 0)


#define _mulle_alloca_do_return_void( name)                       \
do                                                                \
{                                                                 \
   (void) _mulle_alloca_do_free( name);                           \
   return;                                                        \
}                                                                 \
while( 0)


/* API */

/**
 * Reallocates the memory block pointed to by `name` to the specified size .
 * This macro can only be used inside a `mulle_alloca_do` code block.
 *
 * If `name` is currently using the stack-allocated `name__storage` buffer, and
 * the new size exceeds the capacity of the buffer, the memory is reallocated
 * using `mulle_malloc` and the contents of the buffer are copied over.
 *
 * If `name` is already using dynamically allocated memory, it is reallocated
 * using `mulle_realloc`. If the new size is 0, the memory is reallocated to a
 * single element.
 *
 * The `name__count` variable is updated to reflect the new size in elements.
 *
 * @param name The variable name of the memory block to reallocate.
 * @param count The new size in elements to reallocate the memory block to.
 */
//
// Usable only inside of a `mulle_alloca_do` code block.
// If you call `mulle_alloca_do_realloc` with zero, you really get one.
//
#define mulle_alloca_do_realloc( name, count)                     \
do                                                                \
{                                                                 \
	uintptr_t  _count = (count);                                   \
                                                                  \
	if( name == name ## __storage)                                 \
	{                                                              \
      if( _count > _mulle_alloca_do_get_size_as_length( name))    \
		{                                                           \
			name = mulle_malloc( sizeof( *name) * _count);           \
			memcpy( name,                                            \
				     name ## __storage,                               \
				     ((uintptr_t) name ## __count) * sizeof( *name)); \
		}                                                           \
	}                                                              \
	else                                                           \
	{                                                              \
      _count = _count ? _count : 1;                               \
		name   = mulle_realloc( name, _count * sizeof( *name));     \
	}                                                              \
	name ## __count = (void *) _count;                             \
}                                                                 \
while( 0)


/**
 * Extracts the dynamically allocated memory block pointed to by `name` and
 * assigns it to the `receiver` variable. This macro can only be used inside a
 * `mulle_alloca_do` code block.
 *
 * If `name` is currently using the stack-allocated `name__storage` buffer, the
 * memory is reallocated using `mulle_malloc` and the contents of the buffer are
 * copied over before being assigned to `receiver`. If `name` is already using
 * dynamically allocated memory, it is simply assigned to `receiver` and `name`
 * is set to `NULL`.
 *
 * This macro is useful when you need to extract the dynamically allocated
 * memory from the `mulle_alloca_do` block for further processing, while
 * avoiding the need to manually manage the memory allocation.
 *
 * @param name The variable name of the memory block to extract.
 * @param receiver The variable name to assign the extracted memory block to.
 */
/* MEMO: Why is there no `mulle_calloca_do_allocator` ? This is all for
         temporary storage on stack, with fallback to a temporary malloc
         block if needed. If extraction is the main concern of the code,
         wouldn't you "just" mulle_allocator_malloc( allocator, len) ?
         So in effect the scenario for the use of `mulle_alloca_do_extract`
         is a code block, that usually does not extract but sometimes does.
         The extract macro more or less exists, because one gets used to it
         using mulle-container.
 */
#define mulle_alloca_do_extract( name, receiver)                            \
do                                                                          \
{                                                                           \
   if( name == name ## __storage)                                           \
   {                                                                        \
      name = mulle_malloc( ((uintptr_t) name ## __count) * sizeof( *name)); \
      memcpy( name,                                                         \
              name ## __storage,                                            \
              ((uintptr_t) name ## __count) * sizeof( *name));              \
   }                                                                        \
   receiver = name;                                                         \
   name     = NULL;                                                         \
}                                                                           \
while( 0)


/**
 * This macro defines a flexible block of memory that can be allocated either on
 * the stack or dynamically on the heap, depending on the size required. The
 * memory block is allocated using `mulle_malloc` if the requested size exceeds
 * the stack size, or from the stack-allocated `name__storage` array otherwise.
 *
 * The macro takes the following parameters:
 * @param name: The name of the variable that will hold the memory block.
 * @param type: The type of the elements in the memory block.
 * @param count: The number of elements to allocate in the memory block.
 *
 * The macro creates a `for` loop that initializes the memory block and
 * automatically frees it at the end of the loop. The memory block can be
 * accessed using the `name` variable within the loop.
 */
// The "type" is needed for proper stack alignment. Unfortunately something
// array types won't work, you have to use pointers there.
//
// "name__count" is useful for realloc and also to have only one expansion
// point for the macro parameter. Generally all macros in this header have
// only one expansion point, unless noted.
// We need the triple for, because unfortunately when you pass a pointer as
// the type (like struct foo *), the '*' is only applied to the first variable
// of the for variables.
//
#define mulle_alloca_do( name, type, count)                                                          \
   for( type name ## __storage[ _mulle_alloca_stackitems_1( MULLE_ALLOCA_STACKSIZE, type)],          \
        *name ## __count = (void *) (uintptr_t) (count),                                             \
        *name ## __k = NULL;                                                                         \
        ! name ## __k;                                                                               \
        name ## __k++)                                                                               \
      for( type *name = ((uintptr_t) name ## __count) <= _mulle_alloca_do_get_size_as_length( name)  \
                     ? name ## __storage                                                             \
                     : mulle_malloc( sizeof( type) * (uintptr_t) (void *) name ## __count),          \
           *name ## __j = NULL;                                                                      \
           ! name ## __j;                                                                            \
           name ## __j = _mulle_alloca_do_free( name)                                                \
         )                                                                                           \
         for( int  name ## __i = 0;    /* break protection */                                        \
              name ## __i < 1;                                                                       \
              name ## __i++)

/**
 * This macro defines a flexible block of memory that can be allocated either on
 * the stack or dynamically on the heap, depending on the size required. The
 * memory block is allocated using `mulle_malloc` if the requested size exceeds
 * the stack size, or from the stack-allocated `name__storage` array otherwise.
 *
 * The macro takes the following parameters:
 * @param name: The name of the variable that will hold the memory block.
 * @param type: The type of the elements in the memory block.
 * @param stacksize: The initial size of the stack-allocated memory block in bytes.
 * @param count: The number of elements to allocate in the memory block.
 *
 * The macro creates a `for` loop that initializes the memory block and
 * automatically frees it at the end of the loop. The memory block can be
 * accessed using the `name` variable within the loop.
 */
//
// As above, but you can set the initial size in bytes
//
#define mulle_alloca_do_flexible( name, type, stacksize, count)                                      \
   for( type name ## __storage[ _mulle_alloca_stackitems_1( stacksize, type)],                       \
        *name ## __count = (void *) (uintptr_t) (count),                                             \
        *name ## __k = NULL;                                                                         \
        ! name ## __k;                                                                               \
        name ## __k++)                                                                               \
                                                                                                     \
      for( type *name = ((uintptr_t) name ## __count) <= _mulle_alloca_do_get_size_as_length( name)  \
                     ? name ## __storage                                                             \
                     : mulle_malloc( sizeof( type) * (uintptr_t) (void *) name ## __count),          \
           *name ## __j = NULL;                                                                      \
           ! name ## __j;                                                                            \
           name ## __j = _mulle_alloca_do_free( name)                                                \
                                                                                                     \
         )                                                                                           \
         for( int  name ## __i = 0;    /* break protection */                                        \
              name ## __i < 1;                                                                       \
              name ## __i++)


//
// complements mulle_alloca_do
//         
/**
 * This macro defines a block of memory that is allocated on the heap using `mulle_malloc`.
 * The memory block is automatically freed at the end of the loop.
 *
 * The macro takes the following parameters:
 * @param name: The name of the variable that will hold the memory block.
 * @param type: The type of the elements in the memory block.
 * @param count: The number of elements to allocate in the memory block.
 *
 * The macro creates a `for` loop that initializes the memory block and
 * automatically frees it at the end of the loop. The memory block can be
 * accessed using the `name` variable within the loop.
 */
#define mulle_malloc_do( name, type, count)                                 \
   for( type *name = mulle_malloc( sizeof( type) * (count));                \
        name;                                                               \
        name = (mulle_free( name), NULL)                                    \
      )                                                                     \
                                                                            \
      for( int  name ## __i = 0;    /* break protection */                  \
           name ## __i < 1;                                                 \
           name ## __i++)

/**
 * This macro defines a zeroed block of memory that is allocated on the heap using `mulle_calloc`.
 * The memory block is automatically freed at the end of the loop.
 *
 * The macro takes the following parameters:
 * @param name: The name of the variable that will hold the memory block.
 * @param type: The type of the elements in the memory block.
 * @param count: The number of elements to allocate in the memory block.
 *
 * The macro creates a `for` loop that initializes the memory block and
 * automatically frees it at the end of the loop. The memory block can be
 * accessed using the `name` variable within the loop.
 */
 #define mulle_calloc_do( name, type, count)                                \
   for( type *name = mulle_calloc( (count), sizeof( type));                 \
        name;                                                               \
        name = (mulle_free( name), NULL)                                    \
      )                                                                     \
                                                                            \
      for( int  name ## __i = 0;    /* break protection */                  \
           name ## __i < 1;                                                 \
           name ## __i++)



/**
 * This macro defines a loop that iterates over a dynamically allocated buffer.
 *
 * The macro takes the following parameters:
 * @param name: The name of the dynamically allocated buffer.
 * @param len: The number of elements in the buffer.
 * @param p: A pointer variable that will be used to iterate over the buffer.
 *
 * The macro creates a `for` loop that initializes the `p` pointer to the start of
 * the buffer, and iterates until the pointer reaches the end of the buffer. The
 * `p` pointer can be used within the loop to access the elements of the buffer.
 */
// This macro works outside of mulle_alloca_do, you have to give it
// the number of elements in "name" though.
// name and "p" must be a pointer of the correct type.
// name is the mulle_malloced buffer, p can be uninitialized.
//
#define mulle_malloc_for( name, len, p)                         \
   for( __typeof__( name) name ## __sentinel =                  \
         (p = name, &p[ (len)]);                                \
        p < name ## __sentinel;                                 \
        ++p)

/**
 * This macro defines a loop that iterates over a dynamically allocated buffer created by `mulle_alloca_do`.
 *
 * The macro takes the following parameters:
 * @param name: The name of the dynamically allocated buffer.
 * @param p: A pointer variable that will be used to iterate over the buffer.
 *
 * The macro creates a `for` loop that initializes the `p` pointer to the start of
 * the buffer, and iterates until the pointer reaches the end of the buffer. The
 * `p` pointer can be used within the loop to access the elements of the buffer.
 *
 * This macro is intended to be used within the scope of a `mulle_alloca_do` block,
 * where the size of the buffer is known.
 */
//
// This function only works inside mulle_alloca_do
// 'p' needs to be declared outside, for mental consistency with
// mulle_container loops, where this can't be avoided.
//
#define mulle_alloca_do_for( name, p)                           \
   mulle_malloc_for( name, (uintptr_t) name ## __count, p)

// not API and dangerous (s.a.)
#define _mulle_calloca_do_return( name, value)                  \
   _mulle_alloca_do_return( name, value)

#define _mulle_calloca_do_return_void( name)                    \
   _mulle_alloca_do_return_void( name)

#define mulle_calloca_do_extract( name, receiver)               \
   mulle_alloca_do_extract( name, receiver)


/**
 * Reallocates a dynamically allocated buffer created by `mulle_alloca_do`.
 *
 * This macro is used to reallocate a buffer that was originally allocated using
 * `mulle_alloca_do`. It will automatically handle the case where the buffer was
 * allocated on the stack (using `name__storage`) or on the heap (using `mulle_malloc`).
 *
 * If the new size fits within the existing stack-allocated buffer, the macro will
 * simply return without performing any reallocation. Otherwise, it will reallocate
 * the buffer on the heap using `mulle_realloc`.
 *
 * @param name The name of the dynamically allocated buffer to be reallocated.
 * @param count The new size (in elements) for the buffer.
 */
#define mulle_calloca_do_realloc( name, count)                  \
do                                                              \
{                                                               \
   uintptr_t  _count    = (count);                              \
   uintptr_t  _oldcount = (uintptr_t) name ## __count;          \
                                                                \
   if( name == name ## __storage)                               \
   {                                                            \
      if( _count <= _mulle_alloca_do_get_size_as_length( name)) \
         break;                                                 \
                                                                \
      name = mulle_malloc( sizeof( *name) * _count);            \
      memcpy( name,                                             \
              name ## __storage,                                \
              _oldcount * sizeof( *name));                      \
   }                                                            \
   else                                                         \
   {                                                            \
      _count = _count ? _count : 1;                             \
      name   = mulle_realloc( name, _count * sizeof( *name));   \
   }                                                            \
                                                                \
   if( _count > _oldcount)                                      \
      memset( &name[ _oldcount],                                \
              0,                                                \
              sizeof( *name) * (_count -_oldcount));            \
   name ## __count = (void *) _count;                           \
}                                                               \
while( 0)


/**
 * Allocates a dynamically sized zeroed buffer on the stack or heap, depending on the
 * requested size. The buffer is automatically freed at the end of the loop.
 *
 * This macro is used to allocate a buffer that can grow dynamically, but still
 * take advantage of stack-based allocation when possible. It will allocate the
 * buffer on the stack if the requested size fits within a predefined limit
 * (`MULLE_ALLOCA_STACKSIZE`), otherwise it will allocate the buffer on the heap
 * using `mulle_calloc`.
 *
 * The buffer is automatically freed at the end of the loop using the
 * `_mulle_alloca_do_free` function.
 *
 * @param name The name of the dynamically allocated buffer.
 * @param type The type of the elements in the buffer.
 * @param count The initial size (in elements) of the buffer.
 */
#define mulle_calloca_do( name, type, count)                                                         \
   for( type name ## __storage[ _mulle_alloca_stackitems_1( MULLE_ALLOCA_STACKSIZE, type)] = { 0 },  \
        *name ## __count = (void *) (uintptr_t) (count),                                             \
        *name ## __k = NULL;                                                                         \
        ! name ## __k;                                                                               \
        name ## __k++)                                                                               \
      for( type *name = ((uintptr_t) name ## __count) <= _mulle_alloca_do_get_size_as_length( name)  \
                     ? name ## __storage                                                             \
                     : mulle_calloc( (size_t) (void *) name ## __count, sizeof( type)),              \
           *name ## __j = NULL;                                                                      \
           ! name ## __j;                                                                            \
           name ## __j = _mulle_alloca_do_free( name)                                                \
         )                                                                                           \
         for( int  name ## __i = 0;    /* break protection */                                        \
              name ## __i < 1;                                                                       \
              name ## __i++)


/**
 * Allocates a dynamically sized zeroed buffer on the stack or heap, depending on the
 * requested size and a specified stack size limit. The buffer is automatically freed
 * at the end of the loop.
 *
 * This macro is used to allocate a buffer that can grow dynamically, but still
 * take advantage of stack-based allocation when possible. It will allocate the
 * buffer on the stack if the requested size fits within the specified stack size
 * limit, otherwise it will allocate the buffer on the heap using `mulle_calloc`.
 *
 * The buffer is automatically freed at the end of the loop using the
 * `_mulle_alloca_do_free` function.
 *
 * @param name The name of the dynamically allocated buffer.
 * @param type The type of the elements in the buffer.
 * @param stacksize The maximum size (in bytes) of the buffer to allocate on the stack.
 * @param count The initial size (in elements) of the buffer.
 */
//
// as above, but you can set the initial size
//
#define mulle_calloca_do_flexible( name, type, stacksize, count)                                     \
   for( type name ## __storage[ _mulle_alloca_stackitems_1( stacksize, type)] = { 0 },               \
        *name ## __count = (void *) (uintptr_t) (count),                                             \
        *name ## __k = NULL;                                                                         \
        ! name ## __k;                                                                               \
        name ## __k++)                                                                               \
      for( type *name = ((uintptr_t) name ## __count) <= _mulle_alloca_do_get_size_as_length( name)  \
                     ? name ## __storage                                                             \
                     : mulle_calloc( (size_t) (void *) name ## __count, sizeof( type)),              \
           *name ## __j = NULL;                                                                      \
           ! name ## __j;                                                                            \
           name ## __j = _mulle_alloca_do_free( name)                                                \
         )                                                                                           \
         for( int  name ## __i = 0;    /* break protection */                                        \
              name ## __i < 1;                                                                       \
              name ## __i++)


/**
 * Allocates a dynamically sized zeroed buffer on the stack or heap, depending on the
 * requested size and a specified stack size limit. The buffer is automatically freed
 * at the end of the loop.
 *
 * This macro is used to allocate a buffer that can grow dynamically, but still
 * take advantage of stack-based allocation when possible. It will allocate the
 * buffer on the stack if the requested size fits within the specified stack size
 * limit, otherwise it will allocate the buffer on the heap using `mulle_calloc`.
 *
 * The buffer is automatically freed at the end of the loop using the
 * `_mulle_alloca_do_free` function.
 *
 * @param name The name of the dynamically allocated buffer.
 * @param type The type of the elements in the buffer.
 * @param stacksize The maximum size (in bytes) of the buffer to allocate on the stack.
 * @param count The initial size (in elements) of the buffer.
 */
#define mulle_calloc_for( name, len, p) \
   mulle_malloc_for( name, len, p)

/**
 * Allocates a dynamically sized zeroed buffer on the stack or heap, depending on the
 * requested size and a specified stack size limit. The buffer is automatically freed
 * at the end of the loop.
 *
 * This macro is used to allocate a buffer that can grow dynamically, but still
 * take advantage of stack-based allocation when possible. It will allocate the
 * buffer on the stack if the requested size fits within the specified stack size
 * limit, otherwise it will allocate the buffer on the heap using `mulle_calloc`.
 *
 * The buffer is automatically freed at the end of the loop using the
 * `_mulle_alloca_do_free` function.
 *
 * @param name The name of the dynamically allocated buffer.
 * @param type The type of the elements in the buffer.
 * @param stacksize The maximum size (in bytes) of the buffer to allocate on the stack.
 * @param count The initial size (in elements) of the buffer.
 */
//
// This function only works inside mulle_alloca_do
// 'p' needs to be declared outside, for mental consistency with
// mulle_container loops, where this can't be avoided.
//
#define mulle_calloca_do_for( name, p)   \
   mulle_alloca_do_for( name, p)


// backwards compatibility
#define mulle_flexarray_do( a, b, c, d)    mulle_alloca_do( a, b, d)
#define mulle_flexarray_realloc( a, b)     mulle_alloca_do_realloc( a, b)
#define _mulle_flexarray_return( a, b)     _mulle_alloca_do_return( a, b)
#define _mulle_flexarray_return_void( a)   _mulle_alloca_do_return_void( a)

#endif
