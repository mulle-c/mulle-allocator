cmake_minimum_required (VERSION 3.0)

project (mulle-container)


#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -DDEBUG -DMULLE_ABA_TRACE_LIST -DMULLE_ABA_TRACE_SWAP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g -DNDEBUG")

set(HEADERS
src/mulle_allocator.h
)

add_library(mulle_allocator
src/mulle_allocator.c
)

set(TEST_HEADERS
src/mulle_test_allocator.h
)

add_library(mulle_test_allocator
src/mulle_test_allocator.c
)

INSTALL(TARGETS mulle_allocator DESTINATION "lib")
INSTALL(FILES ${HEADERS} DESTINATION "include/mulle_allocator")

INSTALL(TARGETS mulle_test_allocator DESTINATION "lib")
INSTALL(FILES ${TEST_HEADERS} DESTINATION "include/mulle_test_allocator")


