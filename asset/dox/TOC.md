# mulle-allocator Library Documentation for AI
<!-- Keywords: memory, allocator, failfast, stack, fallback, macros, c11 -->

## 1. Introduction & Purpose

`mulle-allocator` is a C memory utility library that provides allocator indirection (`struct mulle_allocator`), fail-fast allocation wrappers, and scoped stack/heap-fallback temporary buffers (`mulle_alloca_do` family). It solves three common problems: hard-wired `malloc` usage in reusable code, repetitive allocation error checks, and unsafe raw `alloca` usage.

At project level, this is a foundational low-level component in the mulle-c ecosystem. It depends directly on `mulle-c11` for portability/compiler macros and exposes a compact public API in headers (`mulle-allocator.h`, `mulle-alloca.h`, `mulle-memset.h`).

## 2. Key Concepts & Design Philosophy

- **Allocator indirection:** `struct mulle_allocator` carries function pointers (`calloc`, `realloc`, `free`, `fail`, `abafree`) plus opaque `aba` context.
- **Fail-fast by default:** allocation wrappers call `fail` callback on failure (default implementation aborts); callers usually do not NULL-check successful paths.
- **Explicit strict realloc variant:** `*_realloc_strict` provides "free-and-NULL" semantics for `size == 0`.
- **Scoped temporary storage macros:** `mulle_alloca_do`/`mulle_calloca_do` use stack for small buffers and heap fallback for larger ones, with automatic cleanup at scope exit.
- **Portable C API surface:** mostly inline wrappers/macros around allocator vectors; no runtime object model.

## 3. Core API & Data Structures

### 3.1. `mulle-allocator-struct.h`

#### `struct mulle_allocator`
- **Purpose:** pluggable allocation vector passed through APIs.
- **Key fields:**
  - `calloc( n, size, allocator)` — allocate zeroed memory.
  - `realloc( block, size, allocator)` — allocate/resize.
  - `free( block, allocator)` — deallocate.
  - `fail( allocator, block, size)` — failure handler (intended no-return).
  - `abafree( aba, free_fn, block, owner)` — optional ABA-aware release.
  - `aba` — opaque ABA context pointer.

### 3.2. `mulle-allocator.h`

#### Global allocators
| Symbol | Short description |
|---|---|
| `mulle_allocator_default` | Default global allocator used by convenience wrappers. |
| `mulle_allocator_stdlib` | Stdlib-backed allocator (`calloc`/`realloc`/`free`). |
| `mulle_allocator_stdlib_nofree` | Stdlib allocation but no-op free function. |
| `mulle_default_allocator` | Legacy macro alias to `mulle_allocator_default`. |
| `mulle_stdlib_allocator` | Legacy macro alias to `mulle_allocator_stdlib`. |
| `mulle_stdlib_nofree_allocator` | Legacy macro alias to `mulle_allocator_stdlib_nofree`. |

#### Standalone functions / declarations
| Symbol | Short description |
|---|---|
| `mulle_allocation_fail` | Default fail callback; reports error then aborts. |
| `mulle_allocator_no_aba_abort` | Default ABA callback; aborts when ABA free is used unconfigured. |
| `mulle_allocator_is_stdlib_allocator` | Checks whether allocator is stdlib-backed (implementation test by function pointer). |
| `_mulle_allocator_realloc_strict` | Core strict realloc helper (`size==0` frees and returns `NULL`). |
| `_mulle_allocator_invalidate` | Overwrites allocator callbacks with abort handlers (test/debug helper). |
| `_mulle_allocator_strdup` | Allocator-based string duplication primitive. |

#### Inline configuration and checks
| Symbol | Short description |
|---|---|
| `mulle_allocator_set_aba` | Sets `aba` context and ABA free callback (or abort default). |
| `mulle_allocator_set_fail` | Sets fail callback (or default fail callback). |
| `mulle_allocator_assert` | Asserts allocator function pointers are present. |

#### Inline allocator API (allocator parameter, `NULL` => default allocator)
| Symbol | Short description |
|---|---|
| `mulle_allocator_malloc` | Allocate bytes; fail callback on allocation failure. |
| `mulle_allocator_calloc` | Allocate zeroed `n * size`; fail callback on failure. |
| `mulle_allocator_realloc` | Resize/allocate; fail callback on failure. |
| `mulle_allocator_realloc_strict` | Strict realloc semantics (`size==0` frees and returns `NULL`). |
| `mulle_allocator_free` | Free block if non-NULL. |
| `mulle_allocator_abafree` | ABA-aware free dispatch; returns status from callback. |
| `mulle_allocator_fail` | Force allocator fail path. |
| `mulle_allocator_strdup` | Duplicate string with selected allocator (`NULL` input => `NULL`). |

#### Inline convenience API (always default allocator)
| Symbol | Short description |
|---|---|
| `mulle_malloc` | Default allocator malloc wrapper. |
| `mulle_calloc` | Default allocator calloc wrapper. |
| `mulle_realloc` | Default allocator realloc wrapper. |
| `mulle_realloc_strict` | Default allocator strict realloc wrapper. |
| `mulle_free` | Default allocator free wrapper. |
| `mulle_abafree` | Default allocator ABA-free wrapper. |
| `mulle_strdup` | Default allocator string duplicate wrapper. |

### 3.3. `mulle-alloca.h`

#### Constants and scoped allocation macros
| Symbol | Short description |
|---|---|
| `MULLE_ALLOCA_STACKSIZE` | Stack/heap threshold in bytes (default `128`). |
| `mulle_alloca_do( name, type, count)` | Scoped temporary buffer; stack first, heap fallback, auto cleanup. |
| `mulle_alloca_do_flexible( name, type, stacksize, count)` | Same as above with per-use stack threshold. |
| `mulle_alloca_do_realloc( name, count)` | Reallocates active `mulle_alloca_do` buffer (inside scope only). |
| `mulle_alloca_do_extract( name, receiver)` | Moves/copies active buffer out of scope ownership. |
| `mulle_alloca_do_for( name, p)` | Iterator helper over active alloca buffer. |
| `mulle_malloc_do( name, type, count)` | Scoped heap allocation wrapper with auto free. |
| `mulle_calloc_do( name, type, count)` | Scoped zeroed heap allocation wrapper with auto free. |
| `mulle_malloc_for( name, len, p)` | Pointer iteration helper for heap buffers. |
| `mulle_calloca_do( name, type, count)` | Zeroed variant of `mulle_alloca_do`. |
| `mulle_calloca_do_flexible( name, type, stacksize, count)` | Flexible zeroed variant. |
| `mulle_calloca_do_realloc( name, count)` | Realloc for `mulle_calloca_do`, zero-fills newly grown tail. |
| `mulle_calloca_do_extract( name, receiver)` | Alias of extract helper. |
| `mulle_calloca_do_for( name, p)` | Alias of iterator helper for `mulle_calloca_do` scope. |
| `mulle_calloc_for( name, len, p)` | Alias to `mulle_malloc_for`. |

#### Compatibility aliases (legacy names)
| Symbol | Short description |
|---|---|
| `mulle_flexarray_do` | Alias to `mulle_alloca_do`. |
| `mulle_flexarray_realloc` | Alias to `mulle_alloca_do_realloc`. |
| `_mulle_flexarray_return` | Alias to `_mulle_alloca_do_return` (advanced/internal). |
| `_mulle_flexarray_return_void` | Alias to `_mulle_alloca_do_return_void` (advanced/internal). |

### 3.4. `mulle-memset.h`

| Symbol | Short description |
|---|---|
| `mulle_memset_uint32( dest, value, count)` | Byte-count fill using repeated 32-bit pattern, including unaligned starts. |

## 4. Performance Characteristics

- **Allocator wrappers (`mulle_*`, `mulle_allocator_*`):** underlying allocator complexity dominates (typically `malloc`/`realloc`/`free` behavior of libc).
- **Wrapper overhead:** O(1) dispatch via function pointer and branch for failure handling.
- **`mulle_alloca_do`/`mulle_calloca_do`:**
  - setup/cleanup O(1),
  - `*_realloc` O(n) when copying from stack storage to heap or when heap realloc moves.
- **`mulle_memset_uint32`:** O(count) byte coverage; unrolled aligned 32-bit writes for larger regions.
- **Thread-safety:**
  - allocation calls are as thread-safe as chosen backend allocator,
  - mutating global allocator vectors (e.g., assigning callbacks on `mulle_allocator_default`) is not synchronized and requires external coordination.

## 5. AI Usage Recommendations & Patterns

### Best practices
1. In reusable libraries, take `struct mulle_allocator *allocator` arguments and route all allocations through `mulle_allocator_*`.
2. Normalize allocator input with `allocator ? allocator : &mulle_allocator_default` only when storing allocator pointers; otherwise wrapper functions already do this.
3. Use `mulle_alloca_do`/`mulle_calloca_do` for temporary buffers whose size may vary significantly.
4. Use `mulle_allocator_realloc_strict` when you need explicit `size==0` free-to-`NULL` semantics.
5. Use `mulle_allocator_set_fail` and `mulle_allocator_set_aba` in tests/specialized runtimes to install custom behavior.

### Common pitfalls
1. Do not describe `mulle_allocator_realloc_strict` as "returns NULL on allocation failure"; failure still routes to fail callback. `NULL` is the defined result for `size==0` (after free).
2. Do not use pointers obtained inside `mulle_alloca_do` outside scope unless extracted with `mulle_alloca_do_extract`.
3. Do not assume `mulle_allocator_stdlib_nofree` frees memory; it intentionally does not.
4. Do not call `mulle_abafree` unless ABA callback/context is configured, or default abort behavior will trigger.

### Idiomatic usage
- Keep allocator pointer in long-lived structs if caller-chosen memory domains must be preserved for destroy/deinit.
- For local scratch memory, prefer scoped macros over manual `malloc/free` pairs.

## 6. Integration Examples

### Example 1: Allocator-parametric lifecycle

```c
#include <mulle-allocator/mulle-allocator.h>
#include <string.h>

struct message
{
   struct mulle_allocator   *allocator;
   char                     *text;
};


static struct message   *message_create( char *s, struct mulle_allocator *allocator)
{
   struct message   *msg;

   allocator = allocator ? allocator : &mulle_allocator_default;
   msg       = mulle_allocator_malloc( allocator, sizeof( struct message));
   msg->text = mulle_allocator_strdup( allocator, s);
   msg->allocator = allocator;
   return( msg);
}


static void   message_destroy( struct message *msg)
{
   if( ! msg)
      return;
   mulle_allocator_free( msg->allocator, msg->text);
   mulle_allocator_free( msg->allocator, msg);
}
```

### Example 2: Scoped temporary buffer with extraction

```c
#include <mulle-allocator/mulle-allocator.h>

static int   *build_table( unsigned int n)
{
   int   *result;
   int   *p;

   result = NULL;
   mulle_alloca_do( values, int, n)
   {
      mulle_alloca_do_for( values, p)
         *p++ = (int) n;

      mulle_alloca_do_extract( values, result);
   }
   return( result);  /* caller frees with mulle_free */
}
```

### Example 3: Pattern fill with 32-bit value

```c
#include <mulle-allocator/mulle-memset.h>
#include <stdint.h>

static void   prepare_guarded_region( uint8_t *buf, size_t len)
{
   mulle_memset_uint32( buf, 0xDEADBEEF, len);
}
```

## 7. Dependencies

- `mulle-c11`
