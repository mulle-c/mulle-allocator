1.6.2
===

* improve documentation


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
