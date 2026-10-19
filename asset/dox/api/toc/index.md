# mulle-allocator Library Documentation for AI
<!-- Keywords: memory, allocator, failfast, stack, fallback, macros, c11 -->

## 1. Introduction & Purpose

`mulle-allocator` is a C memory utility library that provides allocator indirection (`struct mulle_allocator`), fail-fast allocation wrappers, overflow-checked array reallocation (`*_reallocarray*`), checked size-arithmetic helpers, and scoped stack/heap-fallback temporary buffers (`mulle_alloca_do` family). It solves problems such as hard-wired `malloc` usage in reusable code, repetitive allocation error checks, wrapped-around `n * size` allocations, and unsafe raw `alloca` usage.

At project level, this is a foundational low-level component in the mulle-c ecosystem. It depends directly on `mulle-c11` for portability/compiler macros and exposes a compact public API in headers (`mulle-allocator.h`, `mulle-alloca.h`, `mulle-alloca-c23.h`, `mulle-memset.h`).

## 2. Key Concepts & Design Philosophy

- **Allocator indirection:** `struct mulle_allocator` carries function pointers (`calloc`, `reallocarray`, `free`, `fail`, `abafree`) plus opaque `aba` context.
- **Fail-fast by default:** allocation wrappers call `fail` callback on failure (default implementation aborts); callers usually do not NULL-check successful paths.
- **Explicit strict realloc variant:** `*_realloc_strict` and `*_reallocarray_strict` provide "free-and-NULL" semantics for `size == 0` (respectively zero `n`/`size`).
- **Overflow-checked arithmetic:** `mulle_allocator_size_multiply`/`mulle_allocator_size_add` compute sizes for allocation callers; on `size_t` overflow they route through the allocator's `fail` vector (default `mulle_allocation_fail`, which aborts) so wrapped-around sizes never reach the allocator.
- **Array reallocation API:** `*_reallocarray*` resizes `n * size` element arrays. The default vector performs the checked multiplication; a zero `n`/`size` or `n * size` overflow is a programmer error routed to the `fail` vector. The strict variant frees the block and returns `NULL` on zero.
- **Trusted vectors:** the library dispatches straight to the vector functions you install and never validates their arguments (trust "up"); each vector owns its zero/overflow handling. The built-in default vector always rejects a zero `n`/`size` itself and always checks the `count * size` multiplication for `reallocarray` (built on standard `realloc`). For `calloc`, whose stdlib call does the multiplication itself, the extra check is skipped when `MULLE_ALLOCATOR_TRUST_STDLIB` (default `0`) is set to `1`.
- **Scoped temporary storage macros:** `mulle_alloca_do`/`mulle_calloca_do` use stack for small buffers and heap fallback for larger ones, with automatic cleanup at scope exit.
- **Portable C API surface:** mostly inline wrappers/macros around allocator vectors; no runtime object model.

## 3. Core API & Data Structures

### 3.1. `mulle-allocator-struct.h`

#### `struct mulle_allocator`
- **Purpose:** pluggable allocation vector passed through APIs.
- **Key fields:**
  - `calloc( n, size, allocator)` — allocate zeroed memory. The vector must reject zero `n`/`size` and `n * size` overflow itself.
  - `reallocarray( block, n, size, allocator)` — allocate/resize; backs `mulle_malloc`, `mulle_realloc`, `mulle_allocarray` and `mulle_reallocarray` alike. The vector must reject zero `n`/`size` and `n * size` overflow itself.
  - `free( block, allocator)` — deallocate.
  - `fail` (`mulle_allocator_fail_t`) — failure handler. Terminal: must not return (default aborts). The wrapper invokes it on OOM; the default vector also routes programmer errors (zero size, `n * size` overflow) here. It is not a retry hook; recovery belongs in the allocation vectors.
  - `abafree( aba, deferred_free, block, owner)` — optional ABA-aware release. The reclaimer must eventually call `deferred_free( block, owner)` exactly once; mulle-allocator supplies the owning allocator as `owner`, so the block is released through the allocator's own `free`. The reclaimer is trusted like the other vectors.
  - `aba` — opaque ABA context pointer.

#### `mulle_allocator_fail_t`
- **Purpose:** noreturn-qualified function-pointer type of the allocator's `fail` handler (the `fail` field and `mulle_allocator_set_fail`'s parameter).
- **Definition:** `typedef void  (*mulle_allocator_fail_t)( struct mulle_allocator *allocator, void *block, size_t size) MULLE_C_NO_RETURN;`
- Declared as a pointer typedef rather than a function type, so the struct member and `mulle_allocator_set_fail`'s parameter are the exact same qualified type and the assignment needs no cast under `-Werror`.

### 3.2. `mulle-allocator.h`

#### Global allocators
| Symbol | Short description |
|---|---|
| `mulle_allocator_default` | Default global allocator used by convenience wrappers. |
| `mulle_allocator_stdlib` | Stdlib-backed allocator (`calloc`/`reallocarray`/`free`). |
| `mulle_allocator_stdlib_nofree` | Stdlib allocation but no-op free function. |
| `mulle_default_allocator` | Legacy macro alias to `mulle_allocator_default`. |
| `mulle_stdlib_allocator` | Legacy macro alias to `mulle_allocator_stdlib`. |
| `mulle_stdlib_nofree_allocator` | Legacy macro alias to `mulle_allocator_stdlib_nofree`. |

#### Standalone functions / declarations
| Symbol | Short description |
|---|---|
| `mulle_allocation_fail` | Default fail callback; reports error then aborts (never returns). |
| `mulle_allocator_no_aba_abort` | Default ABA callback; prints the missing ABA reclaimer and the remedy (`mulle_aba_init()`), then aborts. |
| `mulle_allocator_is_stdlib_allocator` | Checks whether allocator is stdlib-backed (implementation test by function pointer); takes `const struct mulle_allocator *p`. |
| `_mulle_allocator_realloc_strict` | Core strict realloc helper (`size==0` frees and returns `NULL`). |
| `_mulle_allocator_reallocarray` | Inline core array realloc helper; dispatches to the `reallocarray` vector and routes a NULL result to the `fail` callback. The vector validates `n`/`size`. |
| `_mulle_allocator_reallocarray_strict` | Core strict array realloc helper; if `n * size == 0` frees the block and returns `NULL`. |
| `_mulle_allocator_invalidate` | Overwrites allocator callbacks with abort handlers (test/debug helper). |
| `_mulle_allocator_strdup` | Allocator-based string duplication primitive (takes `const char *s`). |

#### Inline checked size-computation helpers
| Symbol | Short description |
|---|---|
| `mulle_allocator_size_multiply( allocator, a, b)` | Checked `size_t` multiplication (`a * b`); on overflow routes to the allocator's `fail` vector (default aborts). Declared `always_inline`, returns the product. |
| `mulle_allocator_size_add( allocator, a, b)` | Checked `size_t` addition (`a + b`); on wraparound routes to the allocator's `fail` vector (default aborts). Declared `always_inline`, returns the sum. |

Both helpers are intended for consumers of the library when computing allocation sizes; they pass the allocator whose `fail` handler is invoked on overflow. A `NULL` allocator selects the default allocator.

#### Inline configuration and checks
| Symbol | Short description |
|---|---|
| `mulle_allocator_set_aba` | Sets `aba` context and ABA free callback (or abort default). |
| `mulle_allocator_set_fail` | Sets fail callback (or default fail callback). Terminal handler: must not return; not a retry hook. |
| `mulle_allocator_assert` | Asserts allocator function pointers are present. |

#### Inline allocator API (allocator parameter, `NULL` => default allocator)
| Symbol | Short description |
|---|---|
| `mulle_allocator_malloc` | Allocate bytes; fail callback on allocation failure. |
| `mulle_allocator_allocarray` | Allocate an `n * size` array with checked multiply; fail callback on failure. |
| `mulle_allocator_calloc` | Allocate zeroed `n * size`; dispatches to the `calloc` vector and routes a NULL result to the fail callback. The vector validates `n`/`size`. |
| `mulle_allocator_realloc` | Resize/allocate; fail callback on failure. |
| `mulle_allocator_realloc_strict` | Strict realloc semantics (`size==0` frees and returns `NULL`). |
| `mulle_allocator_reallocarray` | Array realloc (`n` elements of `size` bytes); dispatches to the `reallocarray` vector and routes a NULL result to the fail callback. The vector validates `n`/`size`. |
| `mulle_allocator_reallocarray_strict` | Array realloc; if `n * size == 0` frees the block and returns `NULL`. |
| `mulle_allocator_free` | Free block if non-NULL. |
| `mulle_allocator_abafree` | ABA-aware free dispatch; returns zero if `block` is `NULL`, otherwise the ABA reclaimer's status (zero on success). Not a "was the block freed" flag: reclamation may happen asynchronously or later. |
| `mulle_allocator_fail` | Force allocator fail path. |
| `mulle_allocator_strdup` | Duplicate string with selected allocator (`NULL` input => `NULL`). |

#### Inline convenience API (always default allocator)
| Symbol | Short description |
|---|---|
| `mulle_malloc` | Default allocator malloc wrapper. |
| `mulle_allocarray` | Default allocator array allocation wrapper. |
| `mulle_calloc` | Default allocator calloc wrapper. |
| `mulle_realloc` | Default allocator realloc wrapper. |
| `mulle_realloc_strict` | Default allocator strict realloc wrapper. |
| `mulle_reallocarray` | Default allocator array realloc wrapper. |
| `mulle_reallocarray_strict` | Default allocator strict array realloc wrapper. |
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
| `mulle_malloc_do( name, type, count)` | Scoped heap allocation wrapper with auto free. |
| `mulle_calloc_do( name, type, count)` | Scoped zeroed heap allocation wrapper with auto free. |
| `mulle_calloca_do( name, type, count)` | Zeroed variant of `mulle_alloca_do`. |
| `mulle_calloca_do_flexible( name, type, stacksize, count)` | Flexible zeroed variant. |
| `mulle_calloca_do_realloc( name, count)` | Realloc for `mulle_calloca_do`, zero-fills newly grown tail. |
| `mulle_calloca_do_extract( name, receiver)` | Alias of extract helper. |

The `*_for` iterators (`mulle_malloc_for`, `mulle_alloca_do_for`, `mulle_calloc_for`, `mulle_calloca_do_for`) need `typeof` and live in `mulle-alloca-c23.h` (§3.4). This header includes that one automatically when `MULLE_C_HAS_TYPEOF` (from `mulle-c11`) is set.

#### Compatibility aliases (legacy names)
| Symbol | Short description |
|---|---|
| `mulle_flexarray_do` | Alias to `mulle_alloca_do_flexible`; the legacy stack-size argument is a count of `type` elements and is converted to bytes as `(c) * sizeof( b)`. |
| `mulle_flexarray_realloc` | Alias to `mulle_alloca_do_realloc`. |
| `_mulle_flexarray_return_void` | Alias to `_mulle_alloca_do_return_void` (advanced/internal). |

### 3.4. `mulle-alloca-c23.h`

Contains the macros that rely on `__typeof__`/`typeof` (a GNU extension that became standard as `typeof` with C23), so that `mulle-alloca.h` itself stays strict C99/C11 clean. It is included automatically from `mulle-alloca.h` when `MULLE_C_HAS_TYPEOF` (from `mulle-c11/mulle-c11-feature.h`) is set; including it directly on a compiler without `typeof` is an error by design.

| Symbol | Short description |
|---|---|
| `mulle_malloc_for( name, len, p)` | Pointer iteration helper for heap buffers. |
| `mulle_alloca_do_for( name, p)` | Iterator helper over the active `mulle_alloca_do` buffer (uses the scope's element count). |
| `mulle_calloc_for( name, len, p)` | Alias to `mulle_malloc_for`. |
| `mulle_calloca_do_for( name, p)` | Alias of `mulle_alloca_do_for` for the `mulle_calloca_do` scope. |
| `_mulle_alloca_do_return( name, value)` | Value-returning early return from a single `mulle_alloca_do` scope; needs `typeof` (advanced/internal). |
| `_mulle_calloca_do_return( name, value)` | Alias to `_mulle_alloca_do_return` (advanced/internal). |
| `_mulle_flexarray_return` | Alias to `_mulle_alloca_do_return` (advanced/internal). |

### 3.5. `mulle-memset.h`

| Symbol | Short description |
|---|---|
| `mulle_memset_uint32( dest, value, count)` | Fill `count` bytes so that a `uint32_t` read at a 4-byte boundary yields `value` (native byte order); leading bytes are the rotated image. |

## 4. Performance Characteristics

- **Allocator wrappers (`mulle_*`, `mulle_allocator_*`):** underlying allocator complexity dominates (typically `malloc`/`realloc`/`free` behavior of libc).
- **Wrapper overhead:** O(1) dispatch via function pointer and branch for failure handling.
- **`*_reallocarray*` and size helpers:** O(1) checked multiply/add overhead; the underlying resize is that of the backend `realloc`. The `*_strict` zero-size path is an O(1) free.
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
4. Use `mulle_allocator_realloc_strict` when you need explicit `size==0` free-to-`NULL` semantics; use `mulle_allocator_reallocarray(_strict)` for element-array resizing with overflow-checked `n * size`.
5. Compute allocation sizes with `mulle_allocator_size_multiply`/`mulle_allocator_size_add` when caller-supplied counts could overflow; they route to the `fail` vector (terminal) on overflow instead of wrapping.
6. Use `mulle_allocator_set_fail` and `mulle_allocator_set_aba` in tests/specialized runtimes to install custom behavior.

### Common pitfalls
1. Do not describe `mulle_allocator_realloc_strict` as "returns NULL on allocation failure"; failure still routes to fail callback. `NULL` is the defined result for `size==0` (after free).
2. Do not pass `n == 0` or `size == 0` to non-strict `mulle_allocator_reallocarray` expecting a free; for the default vector zero and `n * size` overflow are programming errors routed to the `fail` vector. A custom vector must enforce this contract itself. Use the `_strict` variant to get free-and-`NULL` behavior.
3. Do not use pointers obtained inside `mulle_alloca_do` outside scope unless extracted with `mulle_alloca_do_extract`.
4. Do not assume `mulle_allocator_stdlib_nofree` frees memory; it intentionally does not.
5. Do not call `mulle_abafree` unless ABA callback/context is configured, or default abort behavior will trigger.
6. `fail` is terminal: a custom handler must not return (it aborts, exits or longjmps). It is not a retry hook. To survive memory pressure, implement recovery in the `calloc`/`reallocarray` vector, which can free memory and retry before returning. If a handler were to return without aborting, the compiler may have placed a trap after the call.

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

### Example 3: Array reallocation with overflow-checked size

```c
#include <mulle-allocator/mulle-allocator.h>

static int   *grow_int_array( int *p, size_t new_n)
{
   return( mulle_allocator_reallocarray( NULL, p, new_n, sizeof( int)));
   /* overflow of new_n * sizeof(int) (or new_n == 0) triggers the fail
      callback instead of silently wrapping; pass NULL allocator for the
      default allocator */
}


static int   *clear_int_array( int *p)
{
   return( mulle_allocator_reallocarray_strict( NULL, p, 0, sizeof( int)));
   /* strict variant frees the block and returns NULL */
}
```

Contents of the old block (up to the smaller of old/new size) are preserved on grow/shrink, see `test/reallocarray/reallocarray.c`.

### Example 4: Pattern fill with 32-bit value

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
