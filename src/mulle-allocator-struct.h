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


//
// I put the allocator in the back. It's assumed that the actual call of the
// vector will be inline, and that calloc will be called most often and
// that the compiler doesn't then need to reshuffle arguments registers.
// On many ABIs we should be able to get away with just casting "calloc",
// but that's not easy to test...
//
// abafree uses aba as context
//
// Do not change the allocator after spawning threads. While the actual
// vector functions will be threadsafe the struct itself is not protected.
//
//
// fail is NOT marked _MULLE_C_NO_RETURN anymore. A fail handler may return.
// A return means: "I recovered some memory, retry the allocation". The library
// loops on the failed allocation until it succeeds (see the retry loops in the
// _mulle_allocator_... functions). A handler that returns MUST have made
// forward progress (actually freed memory), otherwise the retry loop spins
// forever. The default handler mulle_allocation_fail aborts and never returns,
// so the loop degenerates to a single call for the default case.
//
#define MULLE_ALLOCATOR_BASE                                                              \
   void   *(*calloc)( size_t n, size_t size, struct mulle_allocator *allocator);          \
   void   *(*realloc)( void *block, size_t size, struct mulle_allocator *allocator);      \
   void   (*free)( void *block, struct mulle_allocator *allocator);                       \
   void   (*fail)( struct mulle_allocator *allocator, void *block, size_t size);          \
   int    (*abafree)( void *aba, void (*free)( void *, void *), void *block, void *owner); \
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
