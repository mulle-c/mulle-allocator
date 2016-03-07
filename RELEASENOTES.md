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