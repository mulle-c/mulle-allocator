
# `mulle_allocator`

`mulle_allocator`: a way to pass around the memory scheme du jour. In general
you shouldn't jump through the vectors directly, but use the supplied inline
functions.

The struct looks like this:

```
struct mulle_allocator
{
   void   *(*calloc)( size_t n, size_t size);
   void   *(*realloc)( void *block, size_t size);
   void   (*free)( void *block);
   int    (*abafree)( void *aba, void (*free)( void *), void *block);
   void   *aba;
};
```

The default allocator `mulle_default_allocator` is defined identical to the
stdlib allocator:

```
struct mulle_allocator   mulle_stdlib_allocator =
{
   calloc, realloc, free, abort, NULL
};
```

By default `.aba` and `.abafree` are not available. For some lock-free data
structures you may need to supply a ABA-safe freeing function.

> Recommendation use [mulle-aba](//www.mulle-kybernetik.com/software/git/mulle-aba/).


# `mulle_test_allocator`

## How to use it

Link against `mulle_test_allocator` to search for leaks and mistaken frees in 
your code. And wrap the test allocator around your code like this:

```
int  main( int argc, char *argv[])
{
   mulle_test_allocator_initialize();
   mulle_default_allocator = mulle_test_allocator;
   {
      // do stuff
   }
   mulle_test_allocator_reset();
}
```

> This will check all allocations going through `mulle_allocator_malloc` and
friends. Direct calls to `malloc` can not be tracked.

## Debug Support

Debug support can be turned on and off with environment variables

Variable                       | Description
------------------------------ | ------------------------------------
MULLE_TEST_ALLOCATOR_TRACE     | Trace allocations and deallocations. A value larger than 1, adds a stacktrace to the output (on participating platforms). A value larger than 2 increases the verbosity of the stacktrace.
MULLE_TEST_ALLOCATOR_DONT_FREE | Memory is not actually freed, this can be useful, when reuse of memory makes the trace too confusing. Obviously this can burn memory away quickly.
