//
//  mulle-allocator-struct.h
//  mulle-allocator
//
//  Copyright (c) 2018 Nat! - Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
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
#ifndef mulle_allocator_struct_h__
#define mulle_allocator_struct_h__

#include <mulle-c11/mulle-c11.h>
#include <stddef.h>


// Forward declaration so the deferred-free callback can name the allocator.
struct mulle_allocator;


//
// A deferred free. An ABA reclaimer eventually calls free(block, owner)
// exactly once, once `block` may safely be reclaimed. `owner` is the value
// the caller of the abafree vector supplied; mulle-allocator always supplies
// the owning struct mulle_allocator *.
//
typedef void (*mulle_allocator_deferred_free_t)( void *block, void *owner);


//
// The ABA reclaimer. `aba` is the reclaimer's own opaque context. Like every
// other vector it is trusted: it must eventually release `block` exactly once
// through the supplied free callback. The return value is the reclaimer's
// status (for example thread registration), 0 on success.
//
typedef int   mulle_allocator_aba_t( void *aba,
                                     mulle_allocator_deferred_free_t free,
                                     void *block,
                                     void *owner);
typedef mulle_allocator_aba_t   *mulle_allocator_abafree_t;


//
// I put the allocator in the back. The leading parameters of a vector mimic
// the corresponding standard C function; the allocator is appended. It's
// assumed that the actual call of the vector will be inline, and that
// calloc/reallocarray will be called most often and that the compiler doesn't
// then need to reshuffle argument registers.
//
// abafree uses aba as its opaque context. It hands `block` to an ABA reclaimer
// (identified by aba); the reclaimer must eventually call the supplied
// deferred free callback exactly once, as free( block, owner). mulle-allocator
// supplies the owning allocator as owner, so the block is released through the
// allocator's own `free` vector. The reclaimer is trusted like every other
// vector, and the owning allocator must outlive every deferred block.
//
// Do not change the allocator after spawning threads. While the actual
// vector functions will be threadsafe the struct itself is not protected.
//
//
// calloc and reallocarray are array shaped: they take an element count and an
// element size. The library TRUSTS the vector functions you install (the
// direction "up"): the funnel and the calloc path pass block/n/size straight
// through and never validate them. A vector must therefore reject a zero
// count/size and an overflowing count*size itself. Everything funnels through
// reallocarray: malloc( size) is reallocarray( NULL, 1, size),
// realloc( block, size) is reallocarray( block, 1, size) and
// allocarray( n, size) is reallocarray( NULL, n, size).
//
// The built-in default vector always rejects a zero count/size itself. Because
// reallocarray is built on standard realloc, it performs (and therefore always
// checks) the count*size multiplication. calloc does the multiplication
// itself, so the multiplicative check is skipped when
// MULLE_ALLOCATOR_TRUST_STDLIB is set.
//
// Honour the calloc/reallocarray contract in your own vector. The library does
// not, and cannot, defend against a vector that ignores a zero count or
// overflows internally.
//
//
// fail is terminal: it is called when an allocation can not be satisfied and
// it must NOT return (hence MULLE_C_NO_RETURN on the vector). The default
// handler mulle_allocation_fail prints an error and aborts.
//
// fail is NOT a recovery or retry hook. If you want to survive memory
// pressure, implement that where it belongs: inside your own
// calloc/reallocarray vector. Such a vector knows its own arena, locks and
// free lists, can release memory and retry the request itself before
// returning. The library calls the vector once and only invokes fail if the
// vector ultimately returns NULL. This keeps the failure policy orthogonal,
// avoids retry loops and counters, and removes the livelock and reentrancy
// hazards of recovering from fail.
//
// fail handler contract:
//
//    * fail is invoked only as a consequence of a rejected allocation request
//      (out of memory, or a programmer error), never of a free. It runs on
//      the thread whose allocation failed.
//    * fail must not return. It may abort, exit or longjmp. Returning is
//      undefined behaviour; the compiler may place a trap after the call.
//    * fail runs in an arbitrary context: there may be no autorelease pool or
//      runloop, a lock may be held, and it may be reached from inside a
//      consumer's own free/reclaim path. Do not assume a "clean" context and
//      do not call back into arbitrary application, ObjC or Foundation code.
//    * fail should be allocation free and, where possible, lock free. As it
//      can not recover, a handler normally only logs/diagnoses and aborts.
//    * Programmer errors (zero size, size overflow) are routed through fail
//      by the default vector (see _mulle_allocator_abort). A custom vector
//      decides for itself; if it treats them as fatal it should route through
//      fail the same way. There is nothing to recover, so the handler must not
//      return here either.
//    * In a multithreaded program do not attempt global "garbage collection"
//      from fail. Global reclamation belongs on a separate asynchronous path
//      (a memory pressure notification or a cleanup thread); recovery of
//      thread-local or arena-owned memory belongs in the vector.
//
// The fail handler. It is terminal, so it is stored as a noreturn-qualified
// function pointer. Declaring it as a pointer typedef (not a function type)
// keeps the struct member and mulle_allocator_set_fail's parameter the exact
// same qualified type, so the assignment needs no cast under -Werror.
//
typedef void  (*mulle_allocator_fail_t)( struct mulle_allocator *allocator,
                                         void *block,
                                         size_t size) MULLE_C_NO_RETURN;
//
#define MULLE_ALLOCATOR_BASE                                                                             \
   void   *(*calloc)( size_t n, size_t size, struct mulle_allocator *allocator);                        \
   void   *(*reallocarray)( void *block, size_t n, size_t size, struct mulle_allocator *allocator);     \
   void   (*free)( void *block, struct mulle_allocator *allocator);                                     \
   mulle_allocator_fail_t fail;                                                                         \
   mulle_allocator_abafree_t abafree;                                                                   \
   void   *aba


//
// mulle_allocator: a way to pass around the memory scheme du jour
//                  you can usually leave aba/abafree uninitialized
//
struct mulle_allocator
{
   MULLE_ALLOCATOR_BASE;
};

#endif
