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

