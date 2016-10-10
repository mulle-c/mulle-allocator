# mulle-allocator API

Familiarize yourself with [stdlib memory allocation](//wikipedia.org/wiki/C_dynamic_memory_allocation) first, because **mulle-vararg** is based on it.


## Global Variables

mulle_allocator provides two allocators: `mulle_stdlib_allocator` and
`mulle_default_allocator`.

The default allocator `mulle_default_allocator` is defined identical to the
`mulle_stdlib_allocator` allocator, which is defined like this:

```
struct mulle_allocator   mulle_stdlib_allocator =
{
   calloc, realloc, free, abort, NULL
};
```

Your code should not modify `mulle_stdlib_allocator` and usually you do not use
it directly but `mulle_default_allocator` instead. A use case for
`mulle_stdlib_allocator` is, when you are interfacing with other code, that
expects this memory to be allocated by stdlib.

You can modify `mulle_default_allocator` though. Do that as early as possible,
before the use of the allocator.


## Functions

> Ensure that allocators do not change once they are passsed. A function
> that accepts an allocator may store a reference or a copy of it in a
> created data structure.
> This is valid and intended.

### stdlib interface

mulle_allocator functions in general usage are like the stdlib allocation
functions with a `mulle_? prefix (they will use the `mulle_default_allocator`):

stdlib  | mulle_allocator
--------|--------------------------
malloc  | `mulle_malloc`
calloc  | `mulle_calloc`
realloc | `mulle_realloc`
free    | `mulle_free`
strdup  | `mulle_strdup`

> e.g. `mulle_strdup( "VfL Bochum 1848");`


### allocator interface

If you want to specify a custom allocator, you use the `mulle_allocator` prefix
and a pointer to your allocator as the first parameter (if the pointer is NULL
`mulle_default_allocator` will be used):

stdlib  | mulle_allocator
--------|--------------------------
malloc  | `mulle_allocator_malloc`
calloc  | `mulle_allocator_calloc`
realloc | `mulle_allocator_realloc`
free    | `mulle_allocator_free`
strdup  | `mulle_allocator_strdup`


> e.g. `mulle_allocator_strdup( NULL, "VfL Bochum 1848");`


