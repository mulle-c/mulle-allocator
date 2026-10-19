# 9.0.0






* make C23 code an optional include
* Add infinite loops after `abort()` calls in mulle-allocator.c to prevent non-void functions from falling through, since tcc does not treat `abort()` as noreturn

feature: split ``__typeof__`` macros out so mulle-alloca.h is strict ANSI C11

* mulle-alloca.h no longer requires ``__typeof__`` and compiles as strict ANSI/ISO C11 with extensions off
* new mulle-alloca-c23.h provides ``mulle_malloc_for`,` ``mulle_alloca_do_for`,` ``mulle_calloc_for`,` ``mulle_calloca_do_for`` and the value-returning ``_mulle_alloca_do_return``
* the C23 header is included automatically when ``MULLE_C_HAS_TYPEOF`` is available (gcc, clang, tcc)
* **BREAKING** on compilers without `typeof`, those macros are no longer available via mulle-alloca.h; include mulle-alloca-c23.h explicitly
* ``add_subdirectory`` consumers again inherit the interface include directories, fixing ``<mulle-allocator/...>`` includes









* Terminate via `exit(0)` in the fail handler instead of returning, since the handler is declared ``MULLE_C_NO_RETURN`` and returning triggers an invalid opcode in clang
* Update expected output to stop after the first failure


* Document the ``mulle_allocator_fail_t`` noreturn function-pointer typedef, its definition, and why it is declared as a pointer type
* Clarify that ``mulle_allocator_abafree`` returns zero for `NULL` blocks and otherwise the reclaimer status, which is not a "was freed" flag
* Note that the default ``mulle_allocator_no_aba_abort`` callback prints the missing reclaimer and the ``mulle_aba_init()`` remedy before aborting
* Explain that ``mulle_flexarray_do`` aliases ``mulle_alloca_do_flexible`` and treats the legacy stack-size argument as an element count converted to bytes



* ``mulle_flexarray_do`` now uses the stack element count the caller passes instead of silently falling back to ``MULLE_ALLOCA_STACKSIZE``
* the default no-ABA reclaimer now explains what is missing, names ``mulle_aba_init()`/`mulle_allocator_set_aba()`` as the remedy, and only then aborts
* documented that abort, exit or longjmp out of the `fail` vector is safe with respect to the allocator, since `fail` is raised after the allocation vector has returned and released its lock







* undid the fail retry branch: `fail` is `MULLE_C_NO_RETURN` again and recovery
  belongs in the calloc/reallocarray vectors, not in the handler
* replaced the `realloc` vector with `reallocarray( block, n, size, allocator)`;
  malloc, realloc, allocarray and reallocarray all funnel through it
* documented the stance: the library trusts the vectors you install, and
  `MULLE_ALLOCATOR_TRUST_STDLIB` only controls the redundant calloc overflow check
* type-correct abafree: a deferred-free callback, no function-pointer punning;
  `mulle_allocator_no_aba_abort` is now `MULLE_C_NO_RETURN`
* calloc overflow now aborts in release builds too
* ABI/API break: allocators built against an earlier version must be recompiled
* added regression tests: vector trust, strict-realloc NULL, calloc overflow
  via a custom fail, size-multiply/add NULL, alloca realloc overflow, abafree

* fail retry branch merged

* new retry scheme for OS with memory recovery


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
