cmake_minimum_required (VERSION 3.4)

project (mulle-container)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/mulle-configuration)
if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  include( Debug)
else()
  include( Release)
endif()


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


