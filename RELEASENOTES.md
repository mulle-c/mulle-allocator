2.1.5
===

* small fixes in documentation and some build internals

2.1.3
===

* merge community release

2.1.2
===

* fix version check, merge with mulle-nat

2.1.1
===

Add mulle_allocator_set_fail for niceness

2.0.2
===

* fix stuff for community release

2.0.0
===

A drastic change. The allocator structure has gained a fail pointe and the
semantics of calloc, realloc are now defined to be unfailing. Check the
README.md for background.

1.8.1
===

* improve .travis.yml

1.8.0
===

* to make it more painless on windows, let `mulle_test_allocator_reset` call
`mulle_test_allocator_initialize` if needed.


1.7.3-1.7.11
===

* improve documentation


1.7.3
===

* more tidying up for release


1.7.2
===

* remove debug flag from release

1.7.1
===

* Improve documentation, put a release branch up


1.7
===

* improve documentation
* improve error output, so that double free is tested first
* add tests


1.6.1
===

* improve documentation
* make it a homebrew package


# 1.6

* test allocator exposes a bit more internal functionality

# 1.5

* improve test realloc, by checking that passed in block is valid

# 1.4

* added mulle_strdup

# 1.3

* fix a bug in pointerset
* added mulle_strdup

# 1.2

* improved the test allocator to use hashtables instead of an array to track
  allocations and frees. As a bonus side effect, it can now also track
  erroneous frees.

# 1.1

* remove smart mode again. Add an aba_free vector, which makes the code using
allocators easier to read IMO (and slightly faster).
* added `mulle_allocator_set_aba`
* added code, so that `mulle_aba_init` automatically updates the default
allocator with itself.

# 1.0

* Add smart mode. malloc routines are then vectored differently. This is useful
to have mulle_aba act as an allocator, making 'aba' transparent to the
data structure.
