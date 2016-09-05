//
//  mulle_test_allocator.h
//  mulle-container
//
//  Created by Nat! on 04.11.15.
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

#ifndef mulle_test_allocator_h__
#define mulle_test_allocator_h__

#include <stddef.h>
#include <mulle_c11/mulle_c11.h>


/*  if you want to replace the default allocator, say
 *    mulle_default_allocator = mulle_test_allocator;
 *  before or after calling `mulle_test_allocator_initialize`
 */
void   mulle_test_allocator_initialize( void);

void   mulle_test_allocator_reset( void);
void   mulle_test_allocator_set_tracelevel( unsigned int value); // 0,1,2

// will be reset by mulle_test_allocator_reset
MULLE_C_EXTERN_GLOBAL int       mulle_test_allocator_out_of_memory;
MULLE_C_EXTERN_GLOBAL size_t    mulle_test_allocator_max_size;

// use this to avoid reuse of memory areas
MULLE_C_EXTERN_GLOBAL int       mulle_test_allocator_dont_free;

MULLE_C_EXTERN_GLOBAL struct mulle_allocator   mulle_test_allocator;

// unlocked functions, rarely useful
void   _mulle_test_allocator_reset( void);
void   _mulle_test_allocator_detect_leaks( void);

#endif /* mulle_test_allocator_h */
