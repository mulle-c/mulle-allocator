# mulle-allocator API

Familiarize yourself with [stdlib memory allocation](//wikipedia.org/wiki/C_dynamic_memory_allocation) first, because **mulle-allocator** is pretty
much the same (and using it).

## How to use it

Link against `mulle_allocator` and
`#include <mulle-allocator/mulle-allocator.h>` in your code.


## Global Variables

**mulle-allocator** provides three allocators: `mulle_allocator_stdlib`,
`mulle_allocator_default` and `mulle_allocator_stdlib_nofree`.

The default allocator `mulle_allocator_default` is initially defined identical
to the `mulle_allocator_stdlib` allocator, which is defined like this:

```
struct mulle_allocator   mulle_allocator_stdlib =
{
   v_calloc, v_reallocarray, v_free, mulle_allocation_fail,
   mulle_allocator_no_aba_abort, NULL
};
```

The leading parameters of a vector mimic the corresponding standard C
function, with a `struct mulle_allocator *` appended. `calloc` is the zeroing
allocator; `reallocarray` backs `mulle_malloc`, `mulle_realloc`,
`mulle_allocarray` and `mulle_reallocarray` alike (see
`mulle-allocator-struct.h` for the exact contract).

> **The library trusts the vectors you install.** The `reallocarray` funnel and
> the `calloc` path pass `block`/`n`/`size` straight through and never validate
> them. A vector must reject a zero `n`/`size` and an overflowing `n * size`
> itself. The built-in `mulle_allocator_default` and `mulle_allocator_stdlib`
> always do; a third-party allocator owns that guarantee. The zero-size and
> overflow behaviour described in this document therefore applies to the
> built-in allocators unless the vector documents otherwise.

Your code should not modify `mulle_allocator_stdlib` and usually you do not use
it directly but you use `mulle_allocator_default` instead. A use case for
`mulle_allocator_stdlib` is, when you are interfacing with other code, that
expects memory to be allocated by stdlib to free or realloc it.

You can modify `mulle_allocator_default`. You must do that before the first use
of `mulle_allocator_default` and before spawning any threads that use it
(allocator setup is single-threaded only).


## Functions

> Ensure that allocators do not change once they are passsed. A function
> that accepts an allocator may store a reference or a copy of it in a
> created data structure. This is valid and intended.


### stdlib interface

**mulle-allocator** functions are just like the stdlib allocation
functions with a `mulle_` prefix (they will use the `mulle_allocator_default`):

| stdlib  | mulle_allocator
|---------|--------------------------
| malloc  | `mulle_malloc`
| calloc  | `mulle_calloc`
| realloc | `mulle_realloc`
| free    | `mulle_free`
| strdup  | `mulle_strdup`

> e.g. `mulle_strdup( "VfL Bochum 1848");`


### allocator interface

If you want to specify a custom allocator, you use the `mulle_allocator_` prefix
and a pointer to your allocator as the first parameter (if the pointer is NULL
`mulle_allocator_default` will be used):

| stdlib  | mulle_allocator
|---------|--------------------------
| malloc  | `mulle_allocator_malloc`
| calloc  | `mulle_allocator_calloc`
| realloc | `mulle_allocator_realloc`
| free    | `mulle_allocator_free`
| strdup  | `mulle_allocator_strdup`


> e.g. `mulle_allocator_strdup( NULL, "VfL Bochum 1848");`


