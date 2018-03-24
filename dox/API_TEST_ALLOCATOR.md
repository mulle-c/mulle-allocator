# `mulle_test_allocator`

## How to use it


Link against `mulle_test_allocator` to search for leaks and mistaken frees in
your code. Wrap the test allocator around your code like this:

```
#include <mulle-allocator/mulle-allocator.h>
#include <mulle-test-allocator/mulle-test-allocator.h>


int  main( int argc, char *argv[])
{
   mulle_test_allocator_initialize(); // or mulle_test_allocator_reset
   mulle_default_allocator = mulle_test_allocator;
   {
      // do stuff
   }
   mulle_test_allocator_reset();
}
```

> This will check all allocations going through `mulle_allocator_malloc` and
friends. Direct calls to `malloc` can not be tracked.


## Environment Variables

Debug support can be turned on and off with environment variables

Variable                         | Description
-------------------------------- | ------------------------------------
`MULLE_TEST_ALLOCATOR_TRACE`     | Trace allocations and deallocations. A value larger than 1, adds a stacktrace to the output (on participating platforms). A value larger than 2 increases the verbosity of the stacktrace.
`MULLE_TEST_ALLOCATOR_DONT_FREE` | Memory is not actually freed, this can be useful, when reuse of memory makes the trace too confusing. Obviously this can burn memory away quickly.

