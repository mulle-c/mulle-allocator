
# mulle_allocator

mulle_allocator: a way to pass around the memory scheme du jour. With the mode
set to one, mulle_allocator uses the smart vectors. In general you shouldn't
jump through the vectors directly, but use the supplied functions.



# mulle_test_allocator

Link this to your tests to search for leaks in your code.

