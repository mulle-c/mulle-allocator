## 8.1.0




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
