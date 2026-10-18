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

// detect the host byte order at runtime, so the expectations below are
// derived from the machine we actually run on
static int host_is_big_endian(void) {
    union {
        uint32_t value;
        uint8_t  byte[sizeof(uint32_t)];
    } probe;

    probe.value = 0x01020304;
    return probe.byte[0] == 0x01;
}

// byte `i` of the 4-byte image of `value` for the requested byte order
static uint8_t image_byte(uint32_t value, size_t i, int big_endian) {
    if (big_endian) {
        return (uint8_t)(value >> ((sizeof(uint32_t) - 1 - i) * 8));
    }
    return (uint8_t)(value >> (i * 8));
}

static uint32_t swap32(uint32_t value) {
    return ((value & 0x000000ffU) << 24) |
           ((value & 0x0000ff00U) << 8)  |
           ((value & 0x00ff0000U) >> 8)  |
           ((value & 0xff000000U) >> 24);
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

// the fill must write the native byte image of `value`, rotated so that the
// bytes at the next 4-byte boundary form the image
int verify_bytes(uint8_t* buf, size_t len, uint32_t pattern, int big_endian) {
    size_t offset = (uintptr_t) buf & (sizeof(uint32_t) - 1);

    for (size_t i = 0; i < len; i++) {
        uint8_t expected = image_byte(pattern, (offset + i) % sizeof(uint32_t), big_endian);
        if (buf[i] != expected) {
            printf("Mismatch at byte %zu: got %02x, expected %02x (%s-endian)\n",
                   i, buf[i], expected, big_endian ? "big" : "little");
            return 0;
        }
    }
    return 1;
}

// the contract: a uint32_t read at a 4-byte boundary yields `value`
int verify_aligned_words(uint8_t* buf, size_t len, uint32_t pattern) {
    uintptr_t start = (uintptr_t) buf;
    uintptr_t addr  = (start + (sizeof(uint32_t) - 1)) & ~(uintptr_t) (sizeof(uint32_t) - 1);
    uintptr_t end   = start + len;

    for (; addr + sizeof(uint32_t) <= end; addr += sizeof(uint32_t)) {
        uint32_t word;
        memcpy(&word, (void*) addr, sizeof(word));
        if (word != pattern) {
            printf("Aligned word at offset %zu is %08x, expected %08x\n",
                   (size_t)(addr - start), word, pattern);
            return 0;
        }
    }
    return 1;
}

int main() {
    const size_t max_size = sizeof(uint64_t) * 4;
    const size_t max_align = sizeof(uint64_t) - 1;
    const uint32_t test_pattern = 0x12345678;
    const int big_endian = host_is_big_endian();

    static uint8_t test_region[sizeof(uint64_t) * 16] = {0};

    for (size_t align = 0; align <= max_align; align++) {
        for (size_t size = 0; size <= max_size; size++) {
            // Clear entire region
            memset(test_region, GUARD_VALUE, sizeof(test_region));
            
            // Calculate our test buffer position with alignment. Round *up*,
            // and start the search one guard below the top of the slack: the
            // buffer is moved forwards by `align` and read back one guard
            // ahead, so the base has to leave GUARD_SIZE bytes of region below
            // it. Rounding the address down (as this did before) puts
            // before_guard outside test_region whenever the array is not
            // already 8-aligned, which is legal - tcc places it at addr % 8
            // == 4.
            uint8_t* aligned_base = (uint8_t*)(((uintptr_t)test_region + max_align + GUARD_SIZE) & ~(uintptr_t)max_align);
            uint8_t* buf = aligned_base + align;
            
            // Guards are everything before buf and everything after buf+size
            uint8_t* before_guard = buf - GUARD_SIZE;
            uint8_t* after_guard = buf + size;
            
            mulle_memset_uint32(buf, test_pattern, size);

            if (!verify_bytes(buf, size, test_pattern, big_endian)) {
                printf("Pattern check failed for alignment %zu, size %zu\n", align, size);
                print_bytes(before_guard, GUARD_SIZE + size + GUARD_SIZE);
                return 1;
            }
            if (!verify_aligned_words(buf, size, test_pattern)) {
                printf("Aligned word check failed for alignment %zu, size %zu\n", align, size);
                print_bytes(before_guard, GUARD_SIZE + size + GUARD_SIZE);
                return 1;
            }
            if (!verify_guards(before_guard, GUARD_SIZE, after_guard, GUARD_SIZE)) {
                printf("Guard check failed for alignment %zu, size %zu\n", align, size);
                print_bytes(before_guard, GUARD_SIZE + size + GUARD_SIZE);
                return 1;
            }
        }
    }

    // Big-endian model: build the big-endian image of the pattern and check
    // that reading it back natively matches the host's byte order. On a
    // big-endian host this is a direct big-endian readback check; on a
    // little-endian host it pins the byte-swapped relationship. The loop above
    // already compares the library against the host's native image, so a
    // big-endian host exercises the library's big-endian path there.
    {
        uint8_t  be_image[sizeof(uint32_t)];
        uint32_t word;

        for (size_t i = 0; i < sizeof(be_image); i++) {
            be_image[i] = image_byte(test_pattern, i, 1);
        }
        memcpy(&word, be_image, sizeof(word));

        if (big_endian) {
            if (word != test_pattern) {
                printf("Big-endian readback mismatch: %08x\n", word);
                return 1;
            }
        } else {
            if (word != swap32(test_pattern)) {
                printf("Little-endian big-endian-model mismatch: %08x\n", word);
                return 1;
            }
        }
    }

    return 0;
}
