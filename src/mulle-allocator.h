//
//  mulle-allocator.c
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
#ifndef mulle_allocator_h__
#define mulle_allocator_h__

#include <mulle-c11/mulle-c11.h>

#ifdef MULLE__ALLOCATOR_BUILD
# define MULLE__ALLOCATOR_GLOBAL    MULLE_C_GLOBAL
#else
# if defined( MULLE_ALLOCATOR_INCLUDE_DYNAMIC) ||  \
     (defined( MULLE_INCLUDE_DYNAMIC) && ! defined( MULLE_ALLOCATOR_INCLUDE_STATIC))
#  define MULLE__ALLOCATOR_GLOBAL   MULLE_C_EXTERN_GLOBAL
# else
#  define MULLE__ALLOCATOR_GLOBAL   extern
# endif
#endif

#include "mulle-allocator-struct.h"
#include <stddef.h>
#include <assert.h>


#if MULLE__C11_VERSION < ((3 << 20) | (0 << 8) | 0)
# error "mulle_c11 is too old"
#endif


#define MULLE__ALLOCATOR_VERSION  ((7UL << 20) | (0 << 8) | 0)


/**
 * `mulle_default_allocator` and `mulle_stdlib_allocator` are global instances of
 * the `mulle_allocator` struct, which provides a standard interface for memory
 * allocation and deallocation.
 *
 * `mulle_default_allocator` is the default allocator used throughout the
 * library. `mulle_stdlib_allocator` is defined to use malloc/calloc/free to
 * interact with other standard library code.
 */
MULLE__ALLOCATOR_GLOBAL struct mulle_allocator   mulle_default_allocator;
MULLE__ALLOCATOR_GLOBAL struct mulle_allocator   mulle_stdlib_allocator;

/**
 * `mulle_stdlib_nofree_allocator` is a global instance of the `mulle_allocator`
 * struct that does not free any memory. It can be used as a special allocator
 * that does not perform any deallocation.
 */
MULLE__ALLOCATOR_GLOBAL struct mulle_allocator   mulle_stdlib_nofree_allocator;


/**
 * This function is called when a memory allocation fails. It is used as the
 * default `fail` function for the `mulle_allocator` struct.
 *
 * The `allocator` parameter is the allocator that failed to allocate the
 * requested memory. The `block` parameter is the memory block that failed to
 * be allocated, and `size` is the size of the requested memory block.
 *
 * This function is marked as `MULLE_C_NO_RETURN`, which means that it will
 * never return.
 */
MULLE_C_NO_RETURN
void   mulle_allocation_fail( struct mulle_allocator *allocator,
                              void *block,
                              size_t size);

/**
 * This function is used as the default `abafree` function for the `mulle_allocator`
 * struct when no custom `abafree` function is provided. It simply aborts the
 * program when the `aba` (Address-Based Allocation) functionality is used but
 * no custom `abafree` function is set.
 *
 * The `aba` parameter is the opaque `aba` data associated with the allocator.
 * The `free` parameter is a pointer to the custom `abafree` function that should
 * have been set. The `block` parameter is the memory block that needs to be
 * freed, and the `owner` parameter is the opaque owner data associated with the
 * memory block.
 */
// NO_RETURN really but....
int   mulle_allocator_no_aba_abort( void *aba,
                       void (*free)( void *, void *),
                       void *block,
                       void *owner);


# pragma mark - Petty Accessors
typedef int   mulle_allocator_aba_t( void *,
                                     void (*f)( void *, void *),
                                     void *,
                                     void *);


/**
 * Checks if the given `mulle_allocator` instance is the `mulle_stdlib_nofree_allocator` global instance.
 *
 * @param p The `mulle_allocator` instance to check.
 * @return 1 if the allocator is the `mulle_stdlib_nofree_allocator` instance, 0 otherwise.
 */
int   mulle_allocator_is_stdlib_allocator( struct mulle_allocator *p);



/**
 * Sets the Address-Based Allocation (ABA) data and function for the given `mulle_allocator` instance.
 *
 * The `aba` parameter is an opaque pointer to the ABA data, and the `f` parameter is a function pointer
 * to the custom `abafree` function that should be used to free memory blocks associated with the ABA data.
 * If `f` is `NULL`, the `mulle_allocator_no_aba_abort` function will be used as the default `abafree` function.
 *
 * @param p The `mulle_allocator` instance to set the ABA data and function for.
 * @param aba The opaque ABA data pointer.
 * @param f The custom `abafree` function pointer, or `NULL` to use the default `mulle_allocator_no_aba_abort` function.
 */
static inline void   mulle_allocator_set_aba( struct mulle_allocator *p,
                                              void *aba,
                                              mulle_allocator_aba_t *f)
{
   if( ! p)
      p = &mulle_default_allocator;

   p->aba     = aba;
   p->abafree = f ? f : mulle_allocator_no_aba_abort;
}


typedef void  mulle_allocator_fail_t( struct mulle_allocator *allocator,
                                      void *,
                                      size_t);

/**
 * Sets the failure handler function for the given `mulle_allocator` instance.
 *
 * The `f` parameter is a function pointer to the custom failure handler function that should be used when an allocation fails. If `f` is `NULL`, the `mulle_allocation_fail` function will be used as the default failure handler.
 *
 * @param p The `mulle_allocator` instance to set the failure handler for.
 * @param f The custom failure handler function pointer, or `NULL` to use the default `mulle_allocation_fail` function.
 */
static inline void   mulle_allocator_set_fail( struct mulle_allocator *p,
                                               mulle_allocator_fail_t *f _MULLE_C_NO_RETURN)
{
   if( ! p)
      p = &mulle_default_allocator;

   p->fail = f ? f : mulle_allocation_fail;
}



# pragma mark - Vectoring


MULLE_C_NONNULL_RETURN
static inline void   *
   _mulle_allocator_malloc( struct mulle_allocator *p, size_t size)
{
   void   *q;

   assert( size);

   q = (*p->realloc)( NULL, size, p);
   if( MULLE_C_UNLIKELY( ! q))
      (*p->fail)( p, NULL, size);
   return( q);
}


MULLE_C_NONNULL_RETURN
static inline void   *
   _mulle_allocator_calloc( struct mulle_allocator *p, size_t n, size_t size)
{
   void   *q;

   assert( n && size);

   q = (*p->calloc)( n, size, p);
   if( MULLE_C_UNLIKELY( ! q))
      (*p->fail)( p, NULL, n * size);
   return( q);
}


//
// this reallocs, but doesn't free. If you pass in size 0, you risk failing.
// you can pass in block 0 for malloc
//
MULLE_C_NONNULL_RETURN
static inline void *
   _mulle_allocator_realloc( struct mulle_allocator *p, void *block, size_t size)
{
   void   *q;

   assert( size);

   q = (*p->realloc)( block, size, p);
   if( MULLE_C_UNLIKELY( ! q))
      (*p->fail)( p, block, size);
   return( q);
}


static inline void   _mulle_allocator_free( struct mulle_allocator *p,
                                            void *block)
{
   if( MULLE_C_LIKELY( block != NULL))
      (*p->free)( block, p);
}



MULLE_C_NO_RETURN
static inline void   _mulle_allocator_fail( struct mulle_allocator *p,
                                            void *block,
                                            size_t size)
{
   (*p->fail)( p, block, size);
}


static inline int   _mulle_allocator_abafree( struct mulle_allocator *p,
                                              void *block)
{
   if( MULLE_C_UNLIKELY( block == NULL))
      return( 0);
   return( (*p->abafree)( p->aba, (void (*)( void *, void *)) p->free, block, p));
}


//
// this function is more like the real realloc, but it is guaranteed that
// if you pass in block != 0 and size 0, that you free AND get NULL back
//
MULLE__ALLOCATOR_GLOBAL
void   *_mulle_allocator_realloc_strict( struct mulle_allocator *p,
                                         void *block,
                                         size_t size);


// trash the allocator, all allocation and free routines will abort
MULLE__ALLOCATOR_GLOBAL
void   _mulle_allocator_invalidate( struct mulle_allocator *p);


/**
 * Asserts that the given `mulle_allocator` instance is valid.
 *
 * This function checks that the `calloc`, `realloc`, `free`, and `fail` function
 * pointers in the `mulle_allocator` are not `NULL`. If any of these pointers are
 * `NULL`, the program will abort.
 *
 * @param p The `mulle_allocator` instance to assert.
 */
static inline void   mulle_allocator_assert( struct mulle_allocator *p)
{
   assert( (p ? p : &mulle_default_allocator));
   assert( (p ? p : &mulle_default_allocator)->calloc);
   assert( (p ? p : &mulle_default_allocator)->realloc);
   assert( (p ? p : &mulle_default_allocator)->free);
   assert( (p ? p : &mulle_default_allocator)->fail);
}


# pragma mark - API

/**
 * Allocates a block of memory of the specified size using the given allocator.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the allocation. Can be `NULL` to use the
 *          default allocator.
 * @param size The size of the block of memory to allocate, in bytes.
 * @return A pointer to the allocated block of memory, or `NULL` if the allocation
 *         failed.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_allocator_malloc( struct mulle_allocator *p, size_t size)
{
   return( _mulle_allocator_malloc( p ? p : &mulle_default_allocator, size));
}


/**
 * Allocates a block of memory of the specified size using the given allocator.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the allocation. Can be `NULL` to use the
 *          default allocator.
 * @param n The number of elements to allocate.
 * @param size The size of each element, in bytes.
 * @return A pointer to the allocated block of memory, or `NULL` if the allocation
 *         failed.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_allocator_calloc( struct mulle_allocator *p,
                                              size_t n,
                                              size_t size)
{
   return( _mulle_allocator_calloc( p ? p : &mulle_default_allocator, n, size));
}


/**
 * Reallocates a block of memory using the given allocator.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the reallocation. Can be `NULL` to use the
 *          default allocator.
 * @param block The block of memory to reallocate.
 * @param size The new size of the block of memory, in bytes.
 * @return A pointer to the reallocated block of memory, or `NULL` if the
 *         reallocation failed.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_allocator_realloc( struct mulle_allocator *p,
                                               void *block,
                                               size_t size)
{
   return( _mulle_allocator_realloc( p ? p : &mulle_default_allocator, block, size));
}


/**
 * Reallocates a block of memory using the given allocator, and fails if the
 * reallocation cannot be performed.
 * This function behaves more like the stdlib realloc, as it is guaranteed that
 * if you pass in a block != 0 and size 0, that you free the block AND get NULL back
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 *
 * @param p The allocator to use for the reallocation. Can be `NULL` to use the
 *          default allocator.
 * @param block The block of memory to reallocate.
 * @param size The new size of the block of memory, in bytes.
 * @return A pointer to the reallocated block of memory or NULL if the block was freed.
 */
static inline void   *mulle_allocator_realloc_strict( struct mulle_allocator *p,
                                                      void *block,
                                                      size_t size)
{
   return( _mulle_allocator_realloc_strict( p ? p : &mulle_default_allocator, block, size));
}



/**
 * Frees a block of memory using the given allocator.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the free operation. Can be `NULL` to use the
 *          default allocator.
 * @param block The block of memory to free.
 */
static inline void   mulle_allocator_free( struct mulle_allocator *p,
                                           void *block)
{
   _mulle_allocator_free( p ? p : &mulle_default_allocator, block);
}


/**
 * Frees a block of memory using the given allocator, and returns a flag indicating
 * whether the block was actually freed.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the free operation. Can be `NULL` to use the
 *          default allocator.
 * @param block The block of memory to free.
 * @return A non-zero value if the block was freed, zero if the block was not freed.
 */
static inline int   mulle_allocator_abafree( struct mulle_allocator *p,
                                             void *block)
{
   return( _mulle_allocator_abafree( p ? p : &mulle_default_allocator, block));
}


/**
 * Fails the allocation operation using the given allocator.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * This function is intended to be used as a fallback when an allocation operation
 * fails. It may perform additional actions, such as logging the failure or
 * triggering an error handling mechanism.
 *
 * @param p The allocator to use for the failed allocation operation. Can be `NULL`
 *          to use the default allocator.
 * @param block The block of memory that failed to be allocated.
 * @param size The size of the block of memory that failed to be allocated.
 */
MULLE_C_NO_RETURN
static inline void   mulle_allocator_fail( struct mulle_allocator *p,
                                           void *block,
                                           size_t size)
{
   _mulle_allocator_fail( p ? p : &mulle_default_allocator, block, size);
}



# pragma mark - Convenience API

/**
 * Allocates a block of memory of the specified size using the default allocator.
 *
 * This function is a convenience wrapper around `_mulle_allocator_malloc()` that
 * uses the default allocator (`mulle_default_allocator`).
 *
 * @param size The size of the block of memory to allocate, in bytes.
 * @return A pointer to the allocated block of memory, or `NULL` if the allocation
 *         failed.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_malloc( size_t size)
{
   return( _mulle_allocator_malloc( &mulle_default_allocator, size));
}


/**
 * Allocates a block of memory of the specified size and initializes it to zero
 * using the default allocator.
 *
 * This function is a convenience wrapper around `_mulle_allocator_calloc()` that
 * uses the default allocator (`mulle_default_allocator`).
 *
 * @param n The number of elements to allocate.
 * @param size The size of each element, in bytes.
 * @return A pointer to the allocated block of memory, or `NULL` if the allocation
 *         failed.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_calloc( size_t n, size_t size)
{
   return( _mulle_allocator_calloc( &mulle_default_allocator, n, size));
}


/**
 * Reallocates the specified block of memory to the given size using the default allocator.
 *
 * This function is a convenience wrapper around `_mulle_allocator_realloc()` that
 * uses the default allocator (`mulle_default_allocator`).
 *
 * @param block The block of memory to reallocate.
 * @param size The new size of the block of memory, in bytes.
 * @return A pointer to the reallocated block of memory, or `NULL` if the reallocation
 *         failed.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_realloc( void *block, size_t size)
{
   return( _mulle_allocator_realloc( &mulle_default_allocator, block, size));
}


/**
 * Reallocates the specified block of memory to the given size using the default allocator.
 *
 * This function is a strict version of `mulle_realloc()` that will return `NULL` if the
 * reallocation fails, instead of attempting to return the original block. This can be
 * useful in situations where you need to ensure the reallocation succeeds or the
 * operation fails completely.
 *
 * @param block The block of memory to reallocate.
 * @param size The new size of the block of memory, in bytes.
 * @return A pointer to the reallocated block of memory, or `NULL` if the reallocation
 *         failed.
 */
static inline void   *mulle_realloc_strict( void *block, size_t size)
{
   return( _mulle_allocator_realloc_strict( &mulle_default_allocator, block, size));
}


/**
 * Frees the specified block of memory using the default allocator.
 *
 * This function is a convenience wrapper around `_mulle_allocator_free()` that
 * uses the default allocator (`mulle_default_allocator`).
 *
 * @param block The block of memory to free.
 */
static inline void   mulle_free( void *block)
{
   _mulle_allocator_free( &mulle_default_allocator, block);
}


/**
 * Frees the specified block of memory using the default allocator.
 *
 * This function is a convenience wrapper around `_mulle_allocator_abafree()` that
 * uses the default allocator (`mulle_default_allocator`).
 *
 * @param block The block of memory to free.
 * @return 0 on success, non-zero on failure.
 */
static inline int   mulle_abafree( void *block)
{
   return( _mulle_allocator_abafree( &mulle_default_allocator, block));
}


# pragma mark - strdup convenience

/**
 * Duplicates the specified string using the given allocator.
 *
 * This function allocates a new string that is a copy of the input string `s`,
 * using the specified allocator `p`. If `p` is `NULL`, the default allocator
 * `mulle_default_allocator` is used.
 *
 * @param p The allocator to use for the new string. Can be `NULL` to use the
 *          default allocator.
 * @param s The string to duplicate.
 * @return A newly allocated string that is a copy of `s`, or `NULL` if s
 *         was NULL.
 */
MULLE__ALLOCATOR_GLOBAL
MULLE_C_NONNULL_RETURN
char   *_mulle_allocator_strdup( struct mulle_allocator *p, char *s);


# pragma mark - strdup API

/**
 * Duplicates the specified string using the given allocator.
 *
 * This function allocates a new string that is a copy of the input string `s`,
 * using the specified allocator `p`. If `p` is `NULL`, the default allocator
 * `mulle_default_allocator` is used.
 *
 * @param p The allocator to use for the new string. Can be `NULL` to use the
 *          default allocator.
 * @param s The string to duplicate.
 * @return A newly allocated string that is a copy of `s`, or `NULL` if s was NULL
 */
static inline char   *mulle_allocator_strdup( struct mulle_allocator *p, char *s)
{
   if( ! s)
      return( s);
   return( _mulle_allocator_strdup( p ? p : &mulle_default_allocator, s));
}


/**
 * Duplicates the specified string using the default allocator.
 *
 * This function allocates a new string that is a copy of the input string `s`,
 * using the default allocator `mulle_default_allocator`.
 *
 * @param s The string to duplicate.
 * @return A newly allocated string that is a copy of `s`
 *
 */
static inline char   *mulle_strdup( char *s)
{
   return( mulle_allocator_strdup( &mulle_default_allocator, s));
}


#include "mulle-alloca.h"


#ifdef __has_include
# if __has_include( "_mulle-allocator-versioncheck.h")
#  include "_mulle-allocator-versioncheck.h"
# endif
#endif

#endif
