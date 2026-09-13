## 9.0.0




feature: the `fail` vector is terminal, recovery belongs in the vectors

* the `fail` vector is `MULLE_C_NO_RETURN`: a fail handler must abort, exit or
  longjmp, never return. A returning "retry the allocation" handler was tried
  and dropped, because it invites retry loops, livelocks and reentrancy
* recovering from memory pressure is an allocation strategy and belongs in the
  `calloc`/`reallocarray` vectors, which know their own arena, locks and free
  lists and may release memory and retry before returning. The library calls a
  vector once and only invokes `fail` if that vector ultimately returns NULL
* zero-size and size overflow are programmer errors and are routed through the
  same `fail` vector, so a custom handler observes and can log them too. There
  is nothing to recover and `fail` stays terminal


fix: unify the allocation vectors around `reallocarray`

* `MULLE_ALLOCATOR_BASE` now carries a
  `reallocarray(block, n, size, allocator)` vector instead of
  `realloc(block, size)`. `malloc`, `realloc`, `allocarray` and `reallocarray`
  all funnel through it; `calloc(n, size, allocator)` stays separate, because
  it zeroes
* the leading parameters of a vector mirror the corresponding standard C
  function, with the `struct mulle_allocator *` appended
* new `mulle_allocator_allocarray` / `mulle_allocarray` convenience functions
* this changes the layout and signature of the base struct: allocators built
  against an earlier version must be recompiled


fix: `calloc` size overflow is no longer only an `assert`

* `mulle_calloc` now aborts on size overflow in release builds too, matching
  the `reallocarray` behaviour. Previously an `NDEBUG` build could compute a
  wrapped-around size


fix: type-correct `abafree` (no more function pointer punning)

* the `abafree` vector is now the named type `mulle_allocator_abafree_t` and
  its callback is `mulle_allocator_deferred_free_t`, both in
  `mulle-allocator-struct.h`
* the allocator no longer casts its `free` vector to a foreign function pointer
  type. It hands the reclaimer `_mulle_allocator_deferred_free`, a real
  function of the callback's exact type, which releases the block through the
  owning allocator's `free`
* `mulle_allocator_no_aba_abort` is now declared `MULLE_C_NO_RETURN`; it still
  returns `int` to match the vector, but it aborts and never returns
* `mulle-aba`'s `_mulle_aba_free_owned_pointer` takes `void *aba` as its first
  parameter, so it installs without a cast


fix: review follow-ups

* the `fail` handler now receives the total requested bytes for
  `reallocarray`/`allocarray` too. It was passed the element size, while the
  docs and `calloc` promise the byte size
* `mulle_calloca_do_realloc` records the new element count when a shrink stays
  inside the stack-resident buffer (it was left stale)
* `_mulle_allocator_invalidate` uses typed abort stubs instead of casting
  `abort` to foreign function pointer types, and invalidates `fail` and `aba`
  as well
* removed the dead `NO_STDLIB_TRAMPOLINE_NEEDED` branch, which no longer
  compiled
* corrected the `mulle_allocator_allocarray` and `mulle_calloc_for` doc comments
* the `mulle_alloca_do` once-only guard no longer increments a NULL pointer,
  removing an undefined-behaviour report under `-fsanitize=undefined`


feature: optional trusted-stdlib fast path

* the library trusts the vector functions you install: the reallocarray funnel
  and the calloc path never validate their arguments, so each vector owns its
  zero/overflow handling
* the built-in default vector always rejects a zero count/size itself and
  always checks the `count * size` multiplication for reallocarray, which is
  built on standard `realloc`
* define `MULLE_ALLOCATOR_TRUST_STDLIB` to 1 to skip the redundant overflow
  check for `calloc`, whose stdlib call does the multiplication itself
  (default 0)


### 8.1.0




feature: add reallocarray and `reallocarray_strict` array reallocation API

* new ``mulle_allocator_reallocarray`` and ``mulle_default_allocator`` convenience ``mulle_reallocarray`` for reallocating arrays with size/length overflow checks
* add ``_strict`` variants that free the block and return NULL when `n` or `size` is zero (stdlib-style) instead of being a programming error
* zero-size / zero-count calls to the non-strict variants now trigger the fail vector in all builds, not just debug
* reworked ``mulle_memset_uint32`` fill helper to use `memcpy` for the word bulk, avoiding alignment and aliasing issues




feature: add checked size computation helpers and `add_subdirectory` cmake support

* new ``mulle_allocator_size_multiply`` and ``mulle_allocator_size_add`` inline helpers that fail (via the allocator's fail vector) on overflow before an undersized allocation can be requested
* calloc path now asserts against `size_t` overflow
* libraries can now be consumed via ``add_subdirectory`:` dependencies resolving to cmake targets are linked and exported as INTERFACE, and reflect headers are filtered to the active variant


* added Documentation & Guides section to README with API summary link


### 8.0.1

Various small improvements
