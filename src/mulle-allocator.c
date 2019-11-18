//  mulle-container
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
#include "include-private.h"
#include "mulle-allocator-struct.h"
// #include "mulle_allocator.h"  // don't include for windows
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>


MULLE_C_NO_RETURN
void   mulle_allocation_fail( void *block, size_t size)
{
   perror( "memory allocation:");
   exit( 1);
}


//
// this function exists because gcc is paining me. But this is also "wrong"
// because it's a supposedly int returning function that should be marked as
// "no-return". It's a classical type tragedy.
//
// MULLE_C_NO_RETURN
int   mulle_aba_abort( void *aba, void (*free)( void *), void *block)
{
   abort();
}

#pragma mark -
#pragma mark _mulle_allocator

MULLE_C_GLOBAL struct mulle_allocator   mulle_stdlib_allocator =
{
   calloc, realloc, free, mulle_allocation_fail, mulle_aba_abort, NULL
};


// this will be patched by the foundation

MULLE_C_GLOBAL struct mulle_allocator   mulle_default_allocator =
{
   calloc, realloc, free, mulle_allocation_fail, mulle_aba_abort, NULL
};


static void   no_free( void *ignored)
{
}

MULLE_C_GLOBAL struct mulle_allocator   mulle_stdlib_nofree_allocator =
{
   calloc, realloc, no_free, mulle_allocation_fail, mulle_aba_abort, NULL
};

