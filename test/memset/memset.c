#include <mulle-allocator/mulle-allocator.h>
#include <mulle-allocator/mulle-memset.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define GUARD_VALUE 0xAA
#define GUARD_SIZE 8

void print_bytes(uint8_t* buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
}

int verify_guards(uint8_t* before_guard, size_t before_size, 
                 uint8_t* after_guard, size_t after_size) {
    for (size_t i = 0; i < before_size; i++) {
        if (before_guard[i] != GUARD_VALUE) {
            printf("Before guard violated at offset %zu: %02x\n", i, before_guard[i]);
            return 0;
        }
    }
    for (size_t i = 0; i < after_size; i++) {
        if (after_guard[i] != GUARD_VALUE) {
            printf("After guard violated at offset %zu: %02x\n", i, after_guard[i]);
            return 0;
        }
    }
    return 1;
}

int verify_pattern(uint8_t* buf, size_t len, uint32_t pattern, size_t start_align) {
    uint32_t expected_rot = (pattern >> (start_align * 8)) | (pattern << (32 - (start_align * 8)));
    for (size_t i = 0; i < len; i++) {
        uint8_t expected = expected_rot & 0xFF;
        if (buf[i] != expected) {
            printf("Mismatch at byte %zu: got %02x, expected %02x\n", 
                   i, buf[i], expected);
            return 0;
        }
        expected_rot = (expected_rot >> 8) | (expected_rot << 24);
    }
    return 1;
}

int main() {
    const size_t max_size = sizeof(uint64_t) * 4;
    const size_t max_align = sizeof(uint64_t) - 1;
    const uint32_t test_pattern = 0x12345678;
    
    static uint8_t test_region[sizeof(uint64_t) * 16] = {0};
    
    for (size_t align = 0; align <= max_align; align++) {
        for (size_t size = 0; size <= max_size; size++) {
            // Clear entire region
            memset(test_region, GUARD_VALUE, sizeof(test_region));
            
            // Calculate our test buffer position with alignment
            uint8_t* aligned_base = (uint8_t*)(((uintptr_t)test_region + 8) & ~(uintptr_t)max_align);
            uint8_t* buf = aligned_base + align;
            
            // Guards are everything before buf and everything after buf+size
            uint8_t* before_guard = buf - GUARD_SIZE;
            uint8_t* after_guard = buf + size;
            
            mulle_memset_uint32(buf, test_pattern, size);

            // printf( "s=%td a=%td: ", size, align);
            // print_bytes( test_region, sizeof( test_region));

            if( ! verify_pattern( buf, size, test_pattern, align))
            {
                printf("Pattern check failed for alignment %zu, size %zu\n", align, size);
                print_bytes(before_guard, GUARD_SIZE + size + GUARD_SIZE);
                return 1;
            }
            if (!verify_guards(before_guard, GUARD_SIZE, after_guard, GUARD_SIZE)) 
            {
                printf("Guard check failed for alignment %zu, size %zu\n", align, size);
                print_bytes(before_guard, GUARD_SIZE + size + GUARD_SIZE);
                return 1;
            }
        }
    }
    
    return 0;
}
