cmake_minimum_required (VERSION 3.0)

project( mulle-allocator C)


set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/mulle-configuration/cmake)
if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  include( Debug)
else()
  include( Release)
endif()


set( MULLE_ALLOCATOR_LIBRARY ON)
set( MULLE_TEST_ALLOCATOR_LIBRARY ON)

include( CMakeSourcesAndHeaders.txt)
include( CMakeDependencies.txt)

include_directories( ${INCLUDE_DIRS})


set( PUBLIC_HEADERS
${HEADERS}
CMakeDependencies.txt
)

add_library( mulle_allocator
${SOURCES}
${HEADERS}
)

# PRIVATE is a guess
target_compile_definitions( mulle_allocator
PRIVATE ${MULLE_ALLOCATOR_DEFINITIONS}
)

add_library( mulle_test_allocator
${TEST_SOURCES}
${TEST_HEADERS}
)

# PRIVATE is a guess
target_compile_definitions( mulle_test_allocator
PRIVATE ${MULLE_ALLOCATOR_DEFINITIONS}
)


INSTALL( TARGETS mulle_allocator DESTINATION "lib")
INSTALL( FILES ${PUBLIC_HEADERS} DESTINATION "include/mulle_allocator")

INSTALL( TARGETS mulle_test_allocator DESTINATION "lib")
INSTALL( FILES ${TEST_HEADERS} DESTINATION "include/mulle_test_allocator")

include( MulleTest)

