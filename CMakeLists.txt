cmake_minimum_required( VERSION 3.0)

project( mulle-allocator C)


### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake")

include( Environment)


### Files

include( PreFiles OPTIONAL)

set( MULLE_ALLOCATOR_LIBRARY ON)

#
# Include all non underscored .cmake files in ./cmake
#
file( GLOB LOCAL_CMAKE_INCLUDES
   "cmake/[A-Za-z0-9]*.cmake"
)

foreach( FILE ${LOCAL_CMAKE_INCLUDES} )
   include( "${FILE}")
endforeach()


include_directories( ${INCLUDE_DIRS})

include( PostFiles OPTIONAL)

### Library

include( PreLibrary OPTIONAL)

add_library( "mulle-allocator"
${SOURCES}
${PUBLIC_HEADERS}
${PRIVATE_HEADERS}
${CMAKE_EDITABLE_FILES}
)

if( BUILD_SHARED_LIBS)
   target_link_libraries( mulle-allocator
${DEPENDENCIES_LIBRARIES}
${OS_SPECIFIC_LIBRARIES}
)
endif()


add_library( "mulle-test-allocator"
${TEST_ALLOCATOR_SOURCES}
${TEST_ALLOCATOR_PUBLIC_HEADERS}
)


if( BUILD_SHARED_LIBS)
   target_link_libraries( mulle-allocator
${DEPENDENCIES_LIBRARIES}
${OS_SPECIFIC_LIBRARIES}
)
endif()

include( PostLibrary OPTIONAL)

### Install

include( PreInstall OPTIONAL)

install( TARGETS "mulle-allocator" DESTINATION "lib")
install( FILES ${PUBLIC_HEADERS}  DESTINATION "include/mulle-allocator")
install( FILES ${PRIVATE_HEADERS} DESTINATION "include/mulle-allocator/private")

install( FILES ${CMAKE_INCLUDES}  DESTINATION "include/mulle-allocator/cmake")

#
# the test allocator is not a "real" dependency
# therefore it's left out of cmake, if that becomes a problem, we need to split
# it off and give it it's own project
#
install( TARGETS "mulle-test-allocator" DESTINATION "lib")
install( FILES ${TEST_ALLOCATOR_PUBLIC_HEADERS}  DESTINATION "include/mulle-test-allocator")

include( PostInstall OPTIONAL)
