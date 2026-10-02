# REVIEW-GUIDE.md

A short guide for reviewers. Most of what gets flagged has already been
considered. This lists the deliberate decisions so review time goes to new
things. If a point below is wrong or stale, *that* is worth reporting.

## Deliberate policy (not defects)

- **`fail` is terminal.** The handler must not return: it aborts, exits or
  longjmps. It is not a retry/recovery hook. Recovery from memory pressure
  belongs in the `calloc`/`reallocarray` vector, which can free memory and
  retry before returning. Retry-from-`fail` was tried and dropped (livelock,
  reentrancy). Notice the "longjmp" possibility you can throw an exception
  here and catch it somewhere, your app need not terminate necessarily.
- **Aborting on OOM is intentional.** This library is for programs that own
  their process. For ENOMEM-style handling of one optional allocation, call
  stdlib `malloc` directly for that allocation.
- **The library trusts the vectors you install.** The funnel passes `n`/`size`
  through unvalidated; each vector owns its zero/overflow handling. The
  built-in allocators always reject zero and check overflow.
- **`reallocarray` is built on standard `realloc`, so we always check
  `n * size`.** `MULLE_ALLOCATOR_TRUST_STDLIB` only skips the redundant check
  in `calloc`, whose stdlib call already multiplies.
- **Zero size is a programming error** for the non-strict API; `_strict` gives
  stdlib-style free-and-NULL.
- **Never-NULL contract:** a satisfied allocation never returns NULL; failure
  aborts. `_strict` returns NULL only for its defined zero path.

## Frequently misread

- **Vector signature: stdlib arguments first, allocator appended.** This keeps
  the hot path register-friendly; the order is intentional.
- **`mulle_memset_uint32` is not a repeated `memset`.** It is an aligned,
  rotated pattern fill so a `uint32_t` read at the 4-byte boundary yields
  `value`. It lives here for dependency-graph reasons.
- **`mulle_alloca_do` stores element counts as `void *`.** Documented and
  intentional; it assumes a flat address space.
- **`is_stdlib_allocator` is coarse by design** (it cannot distinguish
  `stdlib_nofree`); documented.

## Known and tracked (no need to re-file)

- `clib.json` pins `mulle-c11` loosely.

