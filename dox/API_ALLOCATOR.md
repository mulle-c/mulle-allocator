# mulle-allocator API

Familiarize yourself with [stdlib memory allocation](//wikipedia.org/wiki/C_dynamic_memory_allocation) first, because **mulle-allocator** is pretty
much the same (and using it).

## How to use it

Link against `mulle_allocator` and
`#include <mulle-allocator/mulle-allocator.h>` in your code.


## Global Variables

**mulle-allocator** provides three allocators: `mulle_allocator_stdlib` and
`mulle_default_allocator` and `mulle_allocator_stdlib_no_free`.

The default allocator `mulle_default_allocator` is initially defined identical
to the `mulle_allocator_stdlib` allocator, which is defined like this:

```
struct mulle_allocator   mulle_allocator_stdlib =
{
   calloc, realloc, free, abort, NULL
};
```

Your code should not modify `mulle_allocator_stdlib` and usually you do not use
it directly but you use `mulle_default_allocator` instead. A use case for
`mulle_allocator_stdlib` is, when you are interfacing with other code, that
expects memory to be allocated by stdlib to free or realloc it.

You can modify `mulle_default_allocator`. You must do that before the first use
`mulle_default_allocator`.


## Functions

> Ensure that allocators do not change once they are passsed. A function
> that accepts an allocator may store a reference or a copy of it in a
> created data structure. This is valid and intended.


### stdlib interface

**mulle-allocator** functions are just like the stdlib allocation
functions with a `mulle_` prefix (they will use the `mulle_default_allocator`):

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
`mulle_default_allocator` will be used):

| stdlib  | mulle_allocator
|---------|--------------------------
| malloc  | `mulle_allocator_malloc`
| calloc  | `mulle_allocator_calloc`
| realloc | `mulle_allocator_realloc`
| free    | `mulle_allocator_free`
| strdup  | `mulle_allocator_strdup`


> e.g. `mulle_allocator_strdup( NULL, "VfL Bochum 1848");`


