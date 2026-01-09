cmake_minimum_required( VERSION 3.13...99.99)

project( test C)

add_subdirectory( ../../../mulle-c11       ${CMAKE_CURRENT_BINARY_DIR}/mulle-c11)
add_subdirectory( ../../../mulle-allocator ${CMAKE_CURRENT_BINARY_DIR}/mulle-allocator)

add_executable( ${PROJECT_NAME} main.c)
set_target_properties( ${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "package"
    SUFFIX ".exe"
)

target_link_libraries(${PROJECT_NAME} PRIVATE mulle-c11)
target_link_libraries(${PROJECT_NAME} PRIVATE mulle-allocator)

