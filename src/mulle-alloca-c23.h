//
//  mulle-alloca-c23.h
//  mulle-allocator
//
//  Copyright (c) 2026 Nat! - Mulle kybernetiK.
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
#ifndef mulle_alloca_c23_h__
#define mulle_alloca_c23_h__


//
// These macros need __typeof__, which is not C11. It is a GNU extension (also
// understood by clang and tcc) that only became standard as `typeof` with C23.
// They therefore live here, so that mulle-alloca.h itself stays C11 clean.
//
// mulle-alloca.h includes this header automatically when MULLE_C_HAS_TYPEOF
// (from mulle-c11/mulle-c11-feature.h) is set. Including it directly on a
// compiler without typeof is an error by design.
//
#ifndef mulle_alloca_h__
# include "mulle-alloca.h"
#endif

#include <mulle-c11/mulle-c23-feature.h>


//
// This macro is only useful to return from a single `mulle_alloca_do`.
// It won't magically do the right thing in nested mulle_alloca_dos.
// So use with caution, better: Don't use! `break` instead and
// `return` outside of `mulle_alloca_do`. Therefore not part of API.
//
#define _mulle_alloca_do_return( name, value)                     \
do                                                                \
{                                                                 \
   MULLE_C_TYPE_OF( value) name ## __tmp = (value);               \
                                                                  \
   (void) _mulle_alloca_do_free( name);                           \
   MULLE_C_CONFINED_RETURN                                        \
   return( name ## __tmp);                                        \
}                                                                 \
while( 0)


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
   for( MULLE_C_TYPE_OF( name) name ## __sentinel =             \
         (p = name, &p[ (len)]);                                \
        p < name ## __sentinel;                                 \
        ++p)

/**
 * This macro defines a loop that iterates over a dynamically allocated buffer created by `mulle_alloca_do`.
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


/**
 * Iterates over a dynamically allocated buffer, like `mulle_malloc_for`.
 *
 * This is an alias of `mulle_malloc_for`; the name reflects that the buffer
 * was created with one of the zeroing `*calloca*` variants.
 *
 * @param name The name of the dynamically allocated buffer.
 * @param len The number of elements in the buffer.
 * @param p A pointer variable that will be used to iterate over the buffer.
 */
#define mulle_calloc_for( name, len, p) \
   mulle_malloc_for( name, len, p)

//
// This function only works inside mulle_alloca_do
// 'p' needs to be declared outside, for mental consistency with
// mulle_container loops, where this can't be avoided.
//
#define mulle_calloca_do_for( name, p)   \
   mulle_alloca_do_for( name, p)


// not API and dangerous (s.a.)
# define _mulle_calloca_do_return( name, value)                 \
   _mulle_alloca_do_return( name, value)


// backwards compatibility
#define _mulle_flexarray_return( a, b)     _mulle_alloca_do_return( a, b)


#endif
