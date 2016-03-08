
# `mulle_allocator`

`mulle_allocator`: a way to pass around the memory scheme du jour. In general you 
shouldn't jump through the vectors directly, but use the supplied functions.
By default `.aba` and `.abafree` are not available. You need to supply your own
ABA-safe freeing funtion, or use [mulle-aba](https://www.mulle-kybernetik.com/software/git/mulle-aba/)


# `mulle_test_allocator`

Link this to your tests to search for leaks in your code.

