//
//  mulle-memset.c
//  mulle-allocator
//
//  Copyright (c) 2025 Nat! - Mulle kybernetiK.
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
#include "mulle-memset.h"

#include <stdint.h>
#include <string.h>


//
// WHY IS THIS HERE ?
//
// `mulle_memset_uint32` is a fill utility (writing a repeating 32-bit
// pattern, as 0xDEADDEAD, into a block of memory). It has no intrinsic
// connection to allocation, but it lives in mulle-allocator because of
// the dependency graph:
//
//   Users of this function: mulle-buffer, mulle-container, mulle-rbtree,
//   mulle-utf32buffer, mulle-vm.
//   Other users who don't depend on mulle-data: mulle-mmap,
//   mulle-testallocator.
//
//   Moving to mulle-data would force new `mulle-c/mulle-data`
//   dependencies on mulle-mmap and mulle-testallocator, which are
//   deliberately lean, single-dependency libraries.
//
//   Moving to mulle-storage is impossible, as mulle-storage depends on
//   mulle-container, which in turn would need to depend on mulle-storage
//   to use this function (a dependency cycle).
//
//   Mulle-allocator is the natural hub: every current user depends on it
//   already. So this utility lives here, by deliberate choice, so that no
//   library has to grow a new heavyweight dependency.
//
//
// Fill a byte range with a repeating 4-byte pattern. The pattern is
// aligned to 4-byte boundaries: for an unaligned `dest` the pattern
// is shifted so that the bytes of a word at a 4-byte boundary always
// read back as `value` in little-endian order. The unaligned prefix
// and the trailing bytes are written byte-wise, the bulk is copied
// with `memcpy` to avoid any alignment or aliasing issues.
//
void   *mulle_memset_uint32( void *dest, uint32_t value, size_t count)
{
   uint8_t   *d8;
   size_t    offset;
   size_t    i;

   d8     = dest;
   offset = (uintptr_t) dest & 3;

   // shift the pattern so that the first byte of `value` is written
   // to the next 4-byte boundary
   for( i = offset; i < 4 && count; i++, count--)
      *d8++ = (uint8_t)( value >> (i * 8));

   // bulk: whole 32-bit words
   while( count >= 4)
   {
      memcpy( d8, &value, sizeof( value));
      d8    += sizeof( value);
      count -= sizeof( value);
   }

   // trailing partial word
   for( i = 0; i < count; i++)
      d8[ i] = (uint8_t)( value >> (i * 8));

   return( dest);
}