# mulle-allocator Library Documentation for AI

## 1. Introduction & Purpose

`mulle-allocator` provides a flexible, indirectly-dispatched memory allocation scheme for C that decouples code from specific memory backends. Its primary purpose is to enable data structures and algorithms to operate with different allocators—such as standard `malloc`, pool allocators, shared memory, or garbage-collected memory—through a common interface defined by the `struct mulle_allocator`.

A distinguishing feature is its philosophy of **fail-fast memory failure handling**. When allocation fails, the allocator's failure callback is invoked rather than returning NULL. By default, this terminates the program with a clear error message. This eliminates pervasive NULL-check boilerplate after every allocation, leading to cleaner, more maintainable code while making memory errors immediately visible during development.

The library also provides `mulle_alloca_do`, a safe, portable stack allocation macro that automatically falls back to heap allocation for requests exceeding a threshold, preventing stack overflow while maintaining efficiency for typical cases.

## 2. Key Concepts & Design Philosophy

**Allocator Indirection**: The `struct mulle_allocator` is a table of function pointers (`malloc`, `realloc`, `free`, `fail` handler) that dispatch all memory operations. By passing an allocator pointer to data structures and functions, code becomes agnostic to the underlying memory scheme.

**Fail-Fast Over Error Checking**: The design assumes that out-of-memory is unrecoverable and fatal in most portable applications. Rather than forcing cascading NULL checks, allocation failures invoke the `fail` callback which terminates immediately with context. This is pragmatic for systems software where allocation failure is catastrophic.

**Default & Custom Allocators**: When `NULL` is passed as the allocator, the global `mulle_allocator_default` is used. Applications can install custom defaults via module initialization or pass per-instance allocators for flexibility. Supported allocators include:
- `mulle_allocator_default`: Wraps standard C library (malloc/free)
- `mulle_allocator_stdlib`: Explicit standard library allocator
- `mulle_allocator_stdlib_nofree`: Allocates but never frees (for static/arena patterns)

**ABA-Safe Reclamation**: The allocator optionally integrates with `mulle-aba` for safe memory reclamation in lock-free concurrent data structures through `abafree` and `aba` fields.

**Allocator Mobility**: Data structures that embed an allocator pointer become movable across memory schemes without recompilation. A vector created with pool allocation can be used identically to one using standard malloc.

## 3. Core API & Data Structures

### 3.1. `mulle-allocator-struct.h`

#### `struct mulle_allocator`
- **Purpose**: Defines a pluggable memory allocation interface via function pointers
- **Key Fields**:
  - `void *(*calloc)(struct mulle_allocator *, size_t count, size_t size)`: Allocates and zero-initializes memory
  - `void *(*realloc)(struct mulle_allocator *, void *ptr, size_t size)`: Resizes or moves allocation
  - `void (*free)(struct mulle_allocator *, void *ptr)`: Deallocates memory
  - `void (*fail)(struct mulle_allocator *, void *block, size_t size)`: Called on allocation failure (marked NO_RETURN)
  - `void (*abafree)(void *aba, void (*f)(void *, void *), void *block, void *owner)`: Optional ABA-safe deallocation callback
  - `void *aba`: Opaque data for ABA-safe reclamation system
  
**Semantics**:
- All function pointers are never NULL; missing capabilities are implemented as no-ops
- The `fail` callback never returns; it terminates the program
- The `abafree` callback is only used if integrating with `mulle-aba` for lock-free data structures

### 3.2. `mulle-allocator.h` - Global Instances & Operations

#### Global Allocator Instances
- `mulle_allocator_default`: The default global allocator; used when NULL allocator is passed
- `mulle_allocator_stdlib`: Standard C library allocator (`malloc`/`realloc`/`free`)
- `mulle_allocator_stdlib_nofree`: Like `stdlib` but `free()` is a no-op; useful for one-shot allocations or when deallocation isn't needed

#### Core Allocator-Dispatch Functions
These take an explicit `struct mulle_allocator *` parameter (first arg) and dispatch through the allocator:

- `void *mulle_allocator_malloc(struct mulle_allocator *allocator, size_t size)`
  - Allocates uninitialized memory
  - On failure, calls `allocator->fail()` (never returns)
  
- `void *mulle_allocator_calloc(struct mulle_allocator *allocator, size_t count, size_t size)`
  - Allocates and zero-initializes `count * size` bytes
  - Checks for integer overflow (`count * size`)
  - On failure, calls `allocator->fail()`
  
- `void *mulle_allocator_realloc(struct mulle_allocator *allocator, void *ptr, size_t size)`
  - Resizes existing allocation or allocates new block if `ptr` is NULL
  - On failure, calls `allocator->fail()`; original block untouched
  
- `void *mulle_allocator_realloc_strict(struct mulle_allocator *allocator, void *ptr, size_t size)`
  - Like `realloc` but returns NULL on failure instead of calling `fail`
  - Use when you need explicit error handling
  
- `void mulle_allocator_free(struct mulle_allocator *allocator, void *ptr)`
  - Deallocates memory allocated by this allocator
  - Safe to call with NULL (no-op)
  
- `char *mulle_allocator_strdup(struct mulle_allocator *allocator, const char *s)`
  - Duplicates string using allocator
  - Returns allocated copy of `s` (or empty string if `s` is NULL)
  
- `char *mulle_allocator_strndup(struct mulle_allocator *allocator, const char *s, size_t n)`
  - Duplicates up to `n` bytes of string
  - Automatically null-terminates result

#### Convenience Default-Allocator Functions
These use `mulle_allocator_default` automatically; equivalent to allocator-specific versions with NULL:

- `void *mulle_malloc(size_t size)`
- `void *mulle_calloc(size_t count, size_t size)`
- `void *mulle_realloc(void *ptr, size_t size)`
- `void *mulle_realloc_strict(void *ptr, size_t size)`
- `void mulle_free(void *ptr)`
- `char *mulle_strdup(const char *s)`
- `char *mulle_strndup(const char *s, size_t n)`

#### Allocator Query & Management
- `int mulle_allocator_is_stdlib_allocator(struct mulle_allocator *p)`
  - Returns 1 if allocator is the `stdlib_nofree` singleton
  
- `void mulle_allocation_fail(struct mulle_allocator *allocator, void *block, size_t size)`
  - Default failure callback; prints error message and exits with status 1
  - Called by all allocation functions on OOM
  
- `int mulle_allocator_no_aba_abort(void *aba, void (*f)(void *, void *), void *block, void *owner)`
  - Default ABA callback; aborts if ABA system used without proper initialization
  - Indicates programming error (ABA system not configured)

### 3.3. `mulle-alloca.h` - Safe Stack/Heap Fallback Allocation

#### `mulle_alloca_do` Macro
```c
mulle_alloca_do(var_name, type, size)
{
    // var_name is available here as type *
    // Automatically freed on scope exit
}
```

**Behavior**:
- If `size <= MULLE_ALLOCA_STACKSIZE` (default 128 bytes): allocates on stack via `alloca()`
- If `size > MULLE_ALLOCA_STACKSIZE`: allocates on heap via `mulle_malloc()`
- Memory is automatically freed on scope exit (stack: naturally; heap: via cleanup macro)
- `var_name` is typed as `type *` and usable throughout the block

**Key Advantages Over Raw `alloca()`**:
- **Stack-overflow safe**: Large allocations automatically fall back to heap
- **Cross-platform portable**: Works on Windows, Linux, macOS
- **RAII-like cleanup**: No manual free required
- **Type-safe**: Allocates `size` bytes but variable is correctly typed

#### `mulle_calloca_do` Macro
```c
mulle_calloca_do(var_name, type, size)
```

Same as `mulle_alloca_do` but zero-initializes the allocated memory.

#### Configuration
- `MULLE_ALLOCA_STACKSIZE`: Threshold (in bytes) for stack vs heap (default 128)
- Set via compiler flag: `-DMULLE_ALLOCA_STACKSIZE=256` before including header

## 4. Performance Characteristics

**Runtime Overhead**:
- Allocator indirection: Single function pointer dereference per allocation (negligible ~1-2 CPU cycles)
- `mulle_alloca_do` for small allocations: Stack allocation speed (sub-nanosecond)
- `mulle_alloca_do` for large allocations: Identical to `mulle_malloc()` performance

**Memory Footprint**:
- `struct mulle_allocator`: ~48-64 bytes (5-6 function pointers + data)
- Embedding allocator in structure: Minimal overhead (one pointer = 8 bytes on 64-bit)
- Stack: No persistent overhead from `mulle_alloca_do`

**Complexity Analysis**:
- `malloc(n)`: O(1) amortized (same as stdlib)
- `realloc(ptr, n)`: O(n) worst case (may copy data); O(1) if in-place resize succeeds
- `free(ptr)`: O(1) amortized
- `mulle_alloca_do`: O(1) for stack; O(n) for heap fallback

**Thread-Safety**:
- `mulle_allocator_stdlib`: Thread-safe if underlying C library malloc is (typically yes on modern systems)
- Custom allocators: Responsible for own thread safety
- Global `mulle_allocator_default`: Can be replaced but not thread-safe during replacement

**CPU Cache Efficiency**:
- Stack allocation (small): L1 cache hit likely
- Heap allocation: Depends on underlying allocator (malloc typically uses fastbins for small blocks)

## 5. AI Usage Recommendations & Patterns

### Best Practices

1. **Embed allocator in reusable data structures**:
   ```c
   struct my_array {
       struct mulle_allocator *allocator;
       void **elements;
       size_t count, capacity;
   };
   ```
   Enables same code to work with multiple memory schemes

2. **Use allocator-specific functions in libraries**:
   ```c
   // In library/data structure code:
   void *p = mulle_allocator_malloc(allocator, size);  // Respects caller's allocator
   
   // In application code:
   int *x = mulle_malloc(1000);  // Uses default allocator
   ```

3. **Never NULL-check allocation results**:
   ```c
   // ❌ Wrong - unnecessary:
   void *p = mulle_malloc(100);
   if (!p) { error(); }
   
   // ✅ Correct:
   void *p = mulle_malloc(100);  // Fail-fast on OOM; code below always executes
   ```
   Only use `mulle_allocator_realloc_strict()` if explicit error handling needed

4. **Prefer `mulle_alloca_do` over system `alloca()`**:
   ```c
   // ✅ Safe and portable:
   mulle_alloca_do(buffer, char, len) {
       process_buffer(buffer);
   }
   
   // ❌ Risky - stack overflow possible:
   char *buffer = alloca(len);  // If len is large, stack overflow!
   ```

5. **Use `stdlib_nofree` for static/arena patterns**:
   ```c
   struct mulle_allocator *alloc = &mulle_allocator_stdlib_nofree;
   // All allocations use malloc, but no free() calls issued
   // Useful for: bootstrap code, static pools, arena allocators
   ```

### Common Pitfalls

1. **Using wrong allocator to free**:
   ```c
   // ❌ Memory corruption:
   void *p = mulle_allocator_malloc(custom_alloc, 100);
   mulle_free(p);  // Using default allocator!
   
   // ✅ Correct:
   mulle_allocator_free(custom_alloc, p);  // Or store allocator in struct
   ```

2. **Scope confusion with `mulle_alloca_do`**:
   ```c
   // ❌ Dangling pointer:
   char *buffer;
   mulle_alloca_do(tmp, char, 100) {
       buffer = tmp;  // Pointer will be invalid outside block!
   }
   use_buffer(buffer);  // Undefined behavior
   
   // ✅ Correct:
   mulle_alloca_do(buffer, char, 100) {
       use_buffer(buffer);  // Only use inside scope
   }
   ```

3. **Forgetting to free custom allocations**:
   ```c
   // Memory leak:
   void *p = mulle_allocator_malloc(custom_alloc, 100);
   // Forgot to call mulle_allocator_free(custom_alloc, p);
   
   // Easy pattern for self-contained structures:
   struct my_object *obj = create_with_allocator(custom_alloc);
   // obj stores allocator, so cleanup knows which allocator to use
   ```

4. **Mixing NULL and non-NULL allocators inconsistently**:
   ```c
   // Confusing - uses different allocators:
   struct my_vector *v1 = create_vector(NULL);        // Uses default
   struct my_vector *v2 = create_vector(custom);      // Uses custom
   // v1 and v2 now use different memory schemes; hard to debug
   ```

### Idiomatic Patterns

**Pattern 1: Self-Managed Data Structure**
```c
struct string_builder {
    struct mulle_allocator *allocator;
    char *buffer;
    size_t size, capacity;
};

struct string_builder *sb_create(struct mulle_allocator *alloc) {
    alloc = alloc ? alloc : &mulle_allocator_default;
    struct string_builder *sb = mulle_allocator_malloc(alloc, sizeof(*sb));
    sb->allocator = alloc;
    sb->buffer = mulle_allocator_malloc(alloc, 64);
    sb->capacity = 64;
    sb->size = 0;
    return sb;
}

void sb_free(struct string_builder *sb) {
    if (!sb) return;
    mulle_allocator_free(sb->allocator, sb->buffer);
    mulle_allocator_free(sb->allocator, sb);
}
```

**Pattern 2: Temporary Large Buffer with Automatic Cleanup**
```c
void process_large_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    rewind(fp);
    
    // Safe even if file_size is huge - falls back to heap
    mulle_alloca_do(buffer, char, file_size) {
        fread(buffer, 1, file_size, fp);
        process_data(buffer, file_size);
    }  // buffer automatically freed here
    
    fclose(fp);
}
```

**Pattern 3: Custom Allocator for Arena/Pool Pattern**
```c
struct my_allocator {
    struct mulle_allocator base;
    void *arena;
    size_t offset, capacity;
};

// User provides custom allocator callbacks to base.malloc, base.realloc, etc.
struct my_allocator pool = { ... };

// Now all data structures using pool.base work with arena allocation:
struct my_vector *v = create_vector(&pool.base);  // Uses arena, not malloc
```

## 6. Integration Examples

### Example 1: Self-Contained String Structure

Demonstrates embedding allocator to maintain allocation context.

*Source: `README.md`*

```c
#include <mulle-allocator/mulle-allocator.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    struct mulle_allocator *allocator;
    char *data;
    size_t length;
} mulle_string;

mulle_string *mulle_string_create(const char *cstr, 
                                   struct mulle_allocator *allocator)
{
    allocator = allocator ? allocator : &mulle_allocator_default;
    
    size_t len = cstr ? strlen(cstr) : 0;
    
    // Allocate structure + string data in one block
    mulle_string *str = mulle_allocator_malloc(allocator, 
                                               sizeof(*str) + len + 1);
    
    str->allocator = allocator;
    str->data = (char *)(str + 1);  // Point to memory after struct
    str->length = len;
    
    if (cstr)
        strcpy(str->data, cstr);
    else
        str->data[0] = '\0';
    
    return str;
}

void mulle_string_destroy(mulle_string *str)
{
    if (!str) return;
    mulle_allocator_free(str->allocator, str);
}

int main(void)
{
    // Create string with default allocator
    mulle_string *s1 = mulle_string_create("Hello, World!", NULL);
    printf("String 1: %s (len: %zu)\n", s1->data, s1->length);
    
    // Create string with stdlib (explicit)
    mulle_string *s2 = mulle_string_create("Another string", 
                                           &mulle_allocator_stdlib);
    printf("String 2: %s (len: %zu)\n", s2->data, s2->length);
    
    // Clean up - each string knows its own allocator
    mulle_string_destroy(s1);
    mulle_string_destroy(s2);
    
    return 0;
}
```

### Example 2: Safe Stack Allocation with Fallback

Shows automatic stack-to-heap transition for buffer management.

*Source: `test/` directory examples*

```c
#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Transform string to uppercase using temporary buffer
char *to_uppercase(const char *input)
{
    if (!input)
        return mulle_strdup("");
    
    size_t len = strlen(input) + 1;
    
    // Stack-allocated for small strings, heap-allocated for large ones
    mulle_alloca_do(temp, char, len)
    {
        for (size_t i = 0; i < len; i++)
            temp[i] = toupper((unsigned char)input[i]);
        
        // Return a persistent copy (uses mulle_malloc)
        return mulle_strdup(temp);
    }
    // temp is freed automatically here
}

int main(void)
{
    // Small string - uses stack
    char *result1 = to_uppercase("hello");
    printf("Result 1: %s\n", result1);
    mulle_free(result1);
    
    // Large string - uses heap fallback automatically
    char large[1000];
    memset(large, 'a', sizeof(large) - 1);
    large[sizeof(large) - 1] = '\0';
    
    char *result2 = to_uppercase(large);
    printf("Result 2: %zu bytes processed\n", strlen(result2));
    mulle_free(result2);
    
    return 0;
}
```

### Example 3: Custom Allocator Integration

Shows how to create and use a custom allocator with failure handling.

```c
#include <mulle-allocator/mulle-allocator.h>
#include <stdio.h>
#include <stdlib.h>

// Example: allocator with allocation counting
static int allocation_count = 0;
static int max_allocations = 10;  // Fail after 10 allocations

void *counting_malloc(struct mulle_allocator *alloc, size_t size)
{
    if (++allocation_count > max_allocations)
        return NULL;  // Signal failure
    return malloc(size);
}

void counting_fail(struct mulle_allocator *alloc, void *block, size_t size)
{
    fprintf(stderr, "Allocation #%d failed: %zu bytes\n", 
            allocation_count, size);
    exit(1);
}

int main(void)
{
    // Create custom allocator
    struct mulle_allocator counting_alloc = {
        .calloc = (void *(*)(struct mulle_allocator *, size_t, size_t))
                  malloc,  // Simplified - real code needs proper implementation
        .realloc = (void *(*)(struct mulle_allocator *, void *, size_t))
                   realloc,
        .free = (void (*)(struct mulle_allocator *, void *))free,
        .fail = counting_fail,
        .abafree = NULL,
        .aba = NULL
    };
    
    printf("Making allocations with custom allocator...\n");
    for (int i = 0; i < 15; i++) {
        int *arr = mulle_allocator_malloc(&counting_alloc, 100);
        printf("Allocation %d succeeded\n", i + 1);
    }
    
    return 0;
}
```

## 7. Dependencies

- `mulle-c11`: For portability macros (NO_RETURN, ALWAYS_INLINE, etc.)

