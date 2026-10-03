# 9.0.0


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
