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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

//
// written by cody
//
void   *mulle_memset_uint32(void *dest, uint32_t value, size_t count) 
{
    uint8_t *d8 = (uint8_t*)dest;
    uint32_t *d32;
    size_t pre_align, main_count, post_count;
    uint32_t rot_value = value;
    uintptr_t addr = (uintptr_t)dest;
    size_t offset = addr & 3;
    
    // Handle small counts with rotated pattern
    if (count < 4) 
    {
        size_t i;
    
        rot_value = (value >> (offset * 8)) | (value << (32 - (offset * 8)));
        for (i = 0; i < count; i++) 
        {
            d8[i] = (uint8_t)(rot_value & 0xFF);
            rot_value = (rot_value >> 8) | (rot_value << 24);
        }
        return dest;
    }
    
    // Pre-align to 4 bytes, using already rotated value
    pre_align = (4 - offset) & 3;
    if (pre_align) 
    {
        rot_value = (value >> (offset * 8)) | (value << (32 - (offset * 8)));
        for (size_t i = 0; i < pre_align; i++) 
        {
            d8[i] = (uint8_t)(rot_value & 0xFF);
            rot_value = (rot_value >> 8) | (rot_value << 24);
        }
        d8 += pre_align;
        count -= pre_align;
    }
    
    // Now use the rotated value for aligned access
    d32 = (uint32_t*)d8;
    
    // Process 8 words at a time
    main_count = count >> 5;
    while (main_count--) {
        d32[0] = rot_value;
        d32[1] = rot_value;
        d32[2] = rot_value;
        d32[3] = rot_value;
        d32[4] = rot_value;
        d32[5] = rot_value;
        d32[6] = rot_value;
        d32[7] = rot_value;
        d32 += 8;
        count -= 32;  // actually superflous        
    }
    
    // Handle remaining 32-bit writes
    main_count = (count & 31) >> 2;
    while (main_count--) {
        *d32++ = rot_value;
        // Maintain pattern rotation for next 4 bytes
    }

    // Handle remaining bytes, continuing the pattern
    d8 = (uint8_t*)d32;
    post_count = count & 3;
    for (size_t i = 0; i < post_count; i++) {
        d8[i] = (uint8_t)(rot_value & 0xFF);
        rot_value = (rot_value >> 8) | (rot_value << 24);
    }
    
    return dest;
}

