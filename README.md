
# `mulle_allocator`

`mulle_allocator`: a way to pass around the memory scheme du jour. In general you 
shouldn't jump through the vectors directly, but use the supplied functions.
By default `.aba` and `.abafree` are not available. For lock-free data 
structures you may need to supply a ABA-safe freeing function.

> Recommendation use [mulle-aba](https://www.mulle-kybernetik.com/software/git/mulle-aba/).


# `mulle_test_allocator`

Link this to your tests to search for leaks in your code.

