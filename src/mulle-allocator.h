//
//  mulle-allocator.h
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
#ifndef mulle_allocator_h__
#define mulle_allocator_h__

#include <mulle-c11/mulle-c11.h>

#if defined( MULLE__ALLOCATOR_BUILD) || defined( MULLE__CORE_BUILD)
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
#include "mulle-memset.h"
#include <stddef.h>
#include <assert.h>


// keep in sync with src/reflect/_mulle-allocator-versioncheck.h
#if MULLE__C11_VERSION < ((4 << 20) | (9 << 8) | 0)
# error "mulle_c11 is too old"
#endif


#define MULLE__ALLOCATOR_VERSION  ((9UL << 20) | (0 << 8) | 0)


//
// The library trusts the vector functions you install (the direction "up").
// The funnel (`_mulle_allocator_reallocarray`, and therefore `malloc`,
// `realloc`, `allocarray` and `reallocarray`) and the calloc path dispatch
// straight to your vector and never inspect or guard its arguments.
// Validating a zero count/size and a `count * size` overflow is the vector's
// responsibility.
//
// Inside the built-in default vector we look the other way, "down" to the
// platform stdlib. A zero count/size is always checked by the default vector
// (the platform might accept it). reallocarray is not standard C, so the
// default vector builds it on realloc, performs the count*size multiplication
// itself and therefore always checks it. calloc does the multiplication
// itself, so that redundant check is skipped when MULLE_ALLOCATOR_TRUST_STDLIB
// is 1 (the default, 0, checks calloc's multiplication as well).
//
#ifndef MULLE_ALLOCATOR_TRUST_STDLIB
# define MULLE_ALLOCATOR_TRUST_STDLIB   0
#endif


/**
 * `mulle_default_allocator` and `mulle_stdlib_allocator` are global instances of
 * the `mulle_allocator` struct, which provides a standard interface for memory
 * allocation and deallocation.
 *
 * `mulle_default_allocator` is the default allocator used throughout the
 * library. `mulle_stdlib_allocator` is defined to use malloc/calloc/free to
 * interact with other standard library code.
 */
MULLE__ALLOCATOR_GLOBAL struct mulle_allocator   mulle_allocator_default;
MULLE__ALLOCATOR_GLOBAL struct mulle_allocator   mulle_allocator_stdlib;


/**
 * `mulle_stdlib_nofree_allocator` is a global instance of the `mulle_allocator`
 * struct that does not free any memory. It can be used as a special allocator
 * that does not perform any deallocation. This can be useful if your function
 * returns either a static string or a malloced string, pass the either
 * &mulle_allocator_stdlib or &mulle_allocator_stdlib_nofree
 * back in **p_allocator and the caller can then just call
 * mulle_allocator_free( *p_allocator, s) and it will free (or not)
 */
MULLE__ALLOCATOR_GLOBAL struct mulle_allocator   mulle_allocator_stdlib_nofree;


#ifndef NO_MULLE_ALLOCATOR_LEGACY
# define mulle_default_allocator         mulle_allocator_default
# define mulle_stdlib_allocator          mulle_allocator_stdlib
# define mulle_stdlib_nofree_allocator   mulle_allocator_stdlib_nofree
#endif


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
MULLE__ALLOCATOR_GLOBAL
MULLE_C_NO_RETURN
void   mulle_allocation_fail( struct mulle_allocator *allocator,
                              void *block,
                              size_t size);

/**
 * The default `abafree` reclaimer. It is used whenever ABA-safe freeing is
 * attempted without installing a real reclaimer, and it simply aborts.
 *
 * It never returns: it is declared `MULLE_C_NO_RETURN` although it returns
 * `int`, to match the `abafree` vector type.
 *
 * @param aba    the opaque ABA context (unused).
 * @param free   the deferred free callback the reclaimer would have called.
 * @param block  the block that was to be released.
 * @param owner  the opaque owner supplied by the allocator.
 */
MULLE__ALLOCATOR_GLOBAL
MULLE_C_NO_RETURN
int   mulle_allocator_no_aba_abort( void *aba,
                                    void (*free)( void *, void *),
                                    void *block,
                                    void *owner);


// # pragma mark - Petty Accessors
// mulle_allocator_aba_t / mulle_allocator_abafree_t live in
// mulle-allocator-struct.h next to the struct they describe.


/**
 * Checks whether the given `mulle_allocator` dispatches to stdlib via the
 * stdlib trampoline.
 *
 * Because the trampoline is shared, this returns non-zero for
 * `mulle_allocator_stdlib`, for `mulle_allocator_stdlib_nofree` (they differ
 * only in their `free` function) and for any other allocator that reuses the
 * same `calloc`. It does not identify `stdlib_nofree` specifically.
 *
 * @param p The `mulle_allocator` instance to check. `NULL` means the default
 *          allocator.
 * @return 1 if the allocator's `calloc` is the stdlib trampoline, 0 otherwise.
 */
MULLE__ALLOCATOR_GLOBAL
int   mulle_allocator_is_stdlib_allocator( const struct mulle_allocator *p);



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
      p = &mulle_allocator_default;

   p->aba     = aba;
   p->abafree = f ? f : mulle_allocator_no_aba_abort;
}


/*
 * Failure handling
 * ----------------
 *
 * `fail` is terminal. It is invoked when an allocation can not be satisfied
 * and it must not return: it aborts, exits or longjmps. The default handler
 * `mulle_allocation_fail` prints an error and aborts.
 *
 * `fail` is NOT a recovery or retry hook. Recovering from memory pressure is
 * an allocation strategy and belongs in the `calloc`/`realloc` vectors, which
 * know their own arena, locks and free lists and can release memory and retry
 * the request themselves. The library calls a vector once and only invokes
 * `fail` if that vector ultimately returns NULL. This keeps the failure
 * policy orthogonal and avoids retry loops, retry counters, livelock and the
 * reentrancy problems of recovering from `fail`.
 *
 * Programmer errors (zero size, size overflow) are validated by the default
 * vector and routed through the same `fail` vector (see
 * `_mulle_allocator_abort`), so a custom handler observes them. A custom
 * vector performs its own validation and decides whether to route through
 * `fail`. There is nothing to recover: the handler is terminal just the same.
 *
 * The handler receives the allocator, the failing block (may be NULL) and the
 * requested size in bytes. For a failed `realloc`, `block` is still owned by
 * the caller and remains valid.
 *
 * See MULLE_ALLOCATOR_BASE in mulle-allocator-struct.h for the full handler
 * contract.
 */
typedef void  mulle_allocator_fail_t( struct mulle_allocator *allocator,
                                      void *,
                                      size_t);

/**
 * Sets the failure handler function for the given `mulle_allocator` instance.
 *
 * The `f` parameter is a function pointer to the custom failure handler function that should be used when an allocation fails. If `f` is `NULL`, the `mulle_allocation_fail` function will be used as the default failure handler.
 *
 * A fail handler is terminal: it must not return, just like the default
 * `mulle_allocation_fail` which prints an error and aborts. Returning from a
 * handler is undefined behaviour. To recover from memory pressure, implement
 * the recovery inside your `calloc`/`realloc` vector instead -- `fail` is not
 * a retry hook.
 *
 * See MULLE_ALLOCATOR_BASE in mulle-allocator-struct.h for the full handler
 * contract.
 *
 * @param p The `mulle_allocator` instance to set the failure handler for.
 * @param f The custom failure handler function pointer, or `NULL` to use the default `mulle_allocation_fail` function.
 */
static inline void   mulle_allocator_set_fail( struct mulle_allocator *p,
                                               mulle_allocator_fail_t *f MULLE_C_NO_RETURN)
{
   if( ! p)
      p = &mulle_allocator_default;

   p->fail = f ? f : mulle_allocation_fail;
}

//
// Programmer-error terminal. Never returns.
//
// Zero-size and size-overflow are not recoverable, but they are still routed
// through the allocator's `fail` vector, so a custom handler observes them.
// Like every other `fail` call it must not return (MULLE_C_NO_RETURN); the
// vector is trusted to honour that.
//
MULLE_C_NO_RETURN
static inline void   _mulle_allocator_abort( struct mulle_allocator *p,
                                             void *block,
                                             size_t size)
{
   // NULL means "the default allocator", exactly like the public API
   if( ! p)
      p = &mulle_allocator_default;
   (*p->fail)( p, block, size);
}


// #pragma mark - Size computation with underflow checking

//
// Used by consumers of this library, not by mulle_allocator itself.
//
// Checked size_t multiplication for allocation size calculations.
// If the product overflows, it is a programmer error and the allocator's `fail`
// vector is consulted via _mulle_allocator_abort (terminal, never retried).
// This prevents wrapped-around sizes from reaching the allocator, where they
// would silently produce an undersized allocation.
MULLE_C_STATIC_ALWAYS_INLINE
size_t
   mulle_allocator_size_multiply( struct mulle_allocator *allocator,
                                  size_t a,
                                  size_t b)
{
   if( a && b > (size_t) -1 / a)
      _mulle_allocator_abort( allocator, NULL, (size_t) -1);
   return( a * b);
}


//
// To be used by consumers of this library, not by mulle_allocator itself.
//
// Checked size_t addition for allocation size calculations.
// If the sum overflows, it is a programmer error and the allocator's `fail`
// vector is consulted via _mulle_allocator_abort (terminal, never retried).
//
MULLE_C_STATIC_ALWAYS_INLINE
size_t
   mulle_allocator_size_add( struct mulle_allocator *allocator,
                             size_t a,
                             size_t b)
{
   size_t  total;

   total = a + b;
   if( total < a)
      _mulle_allocator_abort( allocator, NULL, (size_t) -1);
   return( total);
}



// # pragma mark - Vectoring


MULLE_C_NONNULL_RETURN
static inline void   *
   _mulle_allocator_reallocarray( struct mulle_allocator *p,
                                  void *block,
                                  size_t n,
                                  size_t size)
{
   void   *q;

   // the installed vector is trusted to validate block/n/size
   q = (*p->reallocarray)( block, n, size, p);
   if( MULLE_C_UNLIKELY( ! q))
      (*p->fail)( p, block, mulle_allocator_size_multiply( p, n, size));
   return( q);
}


MULLE_C_NONNULL_RETURN
static inline void   *
   _mulle_allocator_allocarray( struct mulle_allocator *p, size_t n, size_t size)
{
   return( _mulle_allocator_reallocarray( p, NULL, n, size));
}


MULLE_C_NONNULL_RETURN
static inline void   *
   _mulle_allocator_malloc( struct mulle_allocator *p, size_t size)
{
   return( _mulle_allocator_reallocarray( p, NULL, 1, size));
}


MULLE_C_NONNULL_RETURN
static inline void   *
   _mulle_allocator_calloc( struct mulle_allocator *p, size_t n, size_t size)
{
   void   *q;

   // the installed vector is trusted to validate n/size
   q = (*p->calloc)( n, size, p);
   if( MULLE_C_UNLIKELY( ! q))
      (*p->fail)( p, NULL, mulle_allocator_size_multiply( p, n, size));
   return( q);
}


//
// this reallocs, but doesn't free. If you pass in size 0, you get a failure.
// you can pass in block NULL for malloc
//
MULLE_C_NONNULL_RETURN
static inline void *
   _mulle_allocator_realloc( struct mulle_allocator *p, void *block, size_t size)
{
   return( _mulle_allocator_reallocarray( p, block, 1, size));
}


MULLE__ALLOCATOR_GLOBAL
void *  _mulle_allocator_reallocarray_strict( struct mulle_allocator *p,
                                              void *block,
                                              size_t n,
                                              size_t size);

static inline void   _mulle_allocator_free( struct mulle_allocator *p,
                                            void *block)
{
   if( MULLE_C_LIKELY( block != NULL))
      (*p->free)( block, p);
}



//
// Directly invoke the fail handler. The handler must not return (see
// MULLE_ALLOCATOR_BASE in mulle-allocator-struct.h). Raw passthrough.
//
MULLE_C_NO_RETURN
static inline void   _mulle_allocator_fail( struct mulle_allocator *p,
                                            void *block,
                                            size_t size)
{
   if( ! p)
      p = &mulle_allocator_default;
   (*p->fail)( p, block, size);
}


//
// The allocator's deferred-free adapter. Its type is exactly
// mulle_allocator_deferred_free_t, so an ABA reclaimer can call it with no
// cast. `owner` must be the struct mulle_allocator * that owns `block`; the
// block is released through that allocator's `free` vector.
//
MULLE__ALLOCATOR_GLOBAL
void   _mulle_allocator_deferred_free( void *block, void *owner);


static inline int   _mulle_allocator_abafree( struct mulle_allocator *p,
                                              void *block)
{
   if( MULLE_C_UNLIKELY( block == NULL))
      return( 0);
   return( (*p->abafree)( p->aba, _mulle_allocator_deferred_free, block, p));
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
   // don't assert the allocator itself, it may be NULL (meaning the default)
   assert( (p ? p : &mulle_allocator_default)->calloc);
   assert( (p ? p : &mulle_allocator_default)->reallocarray);
   assert( (p ? p : &mulle_allocator_default)->free);
   assert( (p ? p : &mulle_allocator_default)->fail);

   MULLE_C_UNUSED( p);
}


// # pragma mark - API

/**
 * Allocates a block of memory of the specified size using the given allocator.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the allocation. Can be `NULL` to use the
 *          default allocator.
 * @param size The size of the block of memory to allocate, in bytes.
 * @return A pointer to the allocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_allocator_malloc( struct mulle_allocator *p, size_t size)
{
   return( _mulle_allocator_malloc( p ? p : &mulle_allocator_default, size));
}


/**
 * Allocates an array of `n` elements of `size` bytes using the given
 * allocator. This is the allocation counterpart of
 * `mulle_allocator_reallocarray`: `n * size` is checked for overflow by the
 * allocator's `reallocarray` vector.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the allocation. Can be `NULL` to use the
 *          default allocator.
 * @param n The number of elements to allocate.
 * @param size The size of each element, in bytes.
 * @return A pointer to the allocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_allocator_allocarray( struct mulle_allocator *p,
                                                  size_t n,
                                                  size_t size)
{
   return( _mulle_allocator_allocarray( p ? p : &mulle_allocator_default, n, size));
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
 * @return A pointer to the allocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_allocator_calloc( struct mulle_allocator *p,
                                              size_t n,
                                              size_t size)
{
   return( _mulle_allocator_calloc( p ? p : &mulle_allocator_default, n, size));
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
 * @return A pointer to the reallocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_allocator_realloc( struct mulle_allocator *p,
                                               void *block,
                                               size_t size)
{
   return( _mulle_allocator_realloc( p ? p : &mulle_allocator_default, block, size));
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
   return( _mulle_allocator_realloc_strict( p ? p : &mulle_allocator_default, block, size));
}


/**
 * Reallocates a block of memory for an array of `n` elements of `size` bytes
 * using the given allocator.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the reallocation. Can be `NULL` to use the
 *          default allocator.
 * @param block The block of memory to reallocate.
 * @param n The number of elements in the array.
 * @param size The size of each element, in bytes.
 * @return A pointer to the reallocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_allocator_reallocarray( struct mulle_allocator *p,
                                                    void *block,
                                                    size_t n,
                                                    size_t size)
{
   return( _mulle_allocator_reallocarray( p ? p : &mulle_allocator_default, block, n, size));
}



/**
 * Reallocates a block of memory for an array of `n` elements of `size` bytes
 * using the given allocator. This is the strict variant: if `n` or `size` is
 * zero, the block is freed and `NULL` is returned.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the reallocation. Can be `NULL` to use the
 *          default allocator.
 * @param block The block of memory to reallocate.
 * @param n The number of elements in the array.
 * @param size The size of each element, in bytes.
 * @return A pointer to the reallocated block of memory, or `NULL` if the block
 *         was freed.
 */
static inline void   *mulle_allocator_reallocarray_strict( struct mulle_allocator *p,
                                                    void *block,
                                                    size_t n,
                                                    size_t size)
{
   return( _mulle_allocator_reallocarray_strict( p ? p : &mulle_allocator_default, block, n, size));
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
   _mulle_allocator_free( p ? p : &mulle_allocator_default, block);
}


/**
 * Frees a block of memory using the given allocator, handing it to the
 * allocator's ABA reclaimer if one is installed.
 *
 * If the `p` parameter is `NULL`, the default allocator (`mulle_default_allocator`)
 * will be used.
 *
 * @param p The allocator to use for the free operation. Can be `NULL` to use the
 *          default allocator.
 * @param block The block of memory to free.
 * @return Zero if `block` is `NULL`, otherwise the ABA reclaimer's status
 *         (zero on success). This is not a "was the block freed" flag: a
 *         reclaimer may reclaim the block asynchronously or later.
 */
static inline int   mulle_allocator_abafree( struct mulle_allocator *p,
                                             void *block)
{
   return( _mulle_allocator_abafree( p ? p : &mulle_allocator_default, block));
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
   _mulle_allocator_fail( p ? p : &mulle_allocator_default, block, size);
}



// # pragma mark - Convenience API

/**
 * Allocates a block of memory of the specified size using the default allocator.
 *
 * This function is a convenience wrapper around `_mulle_allocator_malloc()` that
 * uses the default allocator (`mulle_default_allocator`).
 *
 * @param size The size of the block of memory to allocate, in bytes.
 * @return A pointer to the allocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_malloc( size_t size)
{
   return( _mulle_allocator_malloc( &mulle_allocator_default, size));
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
 * @return A pointer to the allocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_calloc( size_t n, size_t size)
{
   return( _mulle_allocator_calloc( &mulle_allocator_default, n, size));
}


/**
 * Reallocates the specified block of memory to the given size using the default allocator.
 *
 * This function is a convenience wrapper around `_mulle_allocator_realloc()` that
 * uses the default allocator (`mulle_default_allocator`).
 *
 * @param block The block of memory to reallocate.
 * @param size The new size of the block of memory, in bytes.
 * @return A pointer to the reallocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_realloc( void *block, size_t size)
{
   return( _mulle_allocator_realloc( &mulle_allocator_default, block, size));
}


/**
 * Reallocates a block of memory for an array of `n` elements of `size` bytes
 * using the default allocator.
 *
 * This function is a convenience wrapper around `_mulle_allocator_reallocarray()`
 * that uses the default allocator (`mulle_default_allocator`).
 *
 * @param block The block of memory to reallocate.
 * @param n The number of elements in the array.
 * @param size The size of each element, in bytes.
 * @return A pointer to the reallocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_reallocarray( void *block, size_t n, size_t size)
{
   return( _mulle_allocator_reallocarray( &mulle_allocator_default, block, n, size));
}


/**
 * Allocates an array of `n` elements of `size` bytes using the default
 * allocator. This is the allocation counterpart of `mulle_reallocarray`:
 * `n * size` is checked for overflow by the allocator's `malloc` vector. A
 * zero `n` or `size` is a programmer error and aborts, exactly like
 * `mulle_malloc` of a zero size.
 *
 * @param n The number of elements to allocate.
 * @param size The size of each element, in bytes.
 * @return A pointer to the allocated block of memory. This never returns
 *         `NULL`: if the allocator can not satisfy the request, `fail` aborts.
 */
MULLE_C_NONNULL_RETURN
static inline void   *mulle_allocarray( size_t n, size_t size)
{
   return( _mulle_allocator_allocarray( &mulle_allocator_default, n, size));
}




/**
 * Reallocates the specified block of memory to the given size using the default allocator.
 *
 * This is the strict, stdlib-like variant: if `size` is `0`, `block` is freed
 * and `NULL` is returned. For any non-zero `size` it never returns `NULL`: if
 * the allocator can not satisfy the request, `fail` aborts.
 *
 * @param block The block of memory to reallocate.
 * @param size The new size of the block of memory, in bytes.
 * @return A pointer to the reallocated block of memory, or `NULL` if `size` was
 *         `0` and the block was freed.
 */
static inline void   *mulle_realloc_strict( void *block, size_t size)
{
   return( _mulle_allocator_realloc_strict( &mulle_allocator_default, block, size));
}



/**
 * Reallocates a block of memory for an array of `n` elements of `size` bytes
 * using the default allocator. This is the strict variant: if `n` or `size` is
 * zero, the block is freed and `NULL` is returned.
 *
 * This function is a convenience wrapper around `_mulle_allocator_reallocarray_strict()`
 * that uses the default allocator (`mulle_default_allocator`).
 *
 * @param block The block of memory to reallocate.
 * @param n The number of elements in the array.
 * @param size The size of each element, in bytes.
 * @return A pointer to the reallocated block of memory, or `NULL` if the block
 *         was freed.
 */
static inline void   *mulle_reallocarray_strict( void *block, size_t n, size_t size)
{
   return( _mulle_allocator_reallocarray_strict( &mulle_allocator_default, block, n, size));
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
   _mulle_allocator_free( &mulle_allocator_default, block);
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
   return( _mulle_allocator_abafree( &mulle_allocator_default, block));
}


// # pragma mark - strdup convenience

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
char   *_mulle_allocator_strdup( struct mulle_allocator *p, const char *s);


// # pragma mark - strdup API

/**
 * Duplicates the specified string using the given allocator.
 *
 * This function allocates a new string that is a copy of the input string `s`,
 * using the specified allocator `p`. If `p` is `NULL`, the default allocator
 * `mulle_default_allocator` is used. If `s` is NULL, NULL is returned.
 *
 * @param p The allocator to use for the new string. Can be `NULL` to use the
 *          default allocator.
 * @param s The string to duplicate.
 * @return A newly allocated string that is a copy of `s`, or `NULL` if s was NULL
 */
static inline char   *mulle_allocator_strdup( struct mulle_allocator *p, const char *s)
{
   if( ! s)
      return( NULL);
   return( _mulle_allocator_strdup( p ? p : &mulle_allocator_default, s));
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
static inline char   *mulle_strdup( const char *s)
{
   return( mulle_allocator_strdup( &mulle_allocator_default, s));
}


#include "mulle-alloca.h"


#ifdef __has_include
# if __has_include( "_mulle-allocator-versioncheck.h")
#  include "_mulle-allocator-versioncheck.h"
# endif
#endif

#endif
