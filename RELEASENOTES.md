# 8.0.0






* `mulle_allocator_is_stdlib_allocator` is now exported `(MULLE__ALLOCATOR_GLOBAL)` so external callers can reliably detect the stdlib allocator
* alloca family macros updated to use `MULLE_C_TYPE_OF` and confined return/loop helpers; added `mulle_calloca_do_extract` alias for consistent extraction semantics
