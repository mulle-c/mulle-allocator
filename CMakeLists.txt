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

include( PostLibrary OPTIONAL)


set( STANDALONE_BASE_NAME "mulle-allocator")
set( STANDALONE_NAME "mulle-allocator-standalone")
set( STANDALONE_DEFINITIONS ${MULLE_ALLOCATOR_DEFINITIONS})
set( STANDALONE_DEPENDENCIES
mulle-allocator
)

set( STANDALONE_ALL_LOAD_LIBRARIES
$<TARGET_FILE:mulle-allocator>
${C_DEPENDENCY_LIBRARIES}
${OS_SPECIFIC_LIBRARIES}
)

include( Standalone)

### Install

include( PreInstall OPTIONAL)

install( TARGETS "mulle-allocator" "mulle-allocator-standalone" DESTINATION "lib")
install( FILES ${PUBLIC_HEADERS}  DESTINATION "include/mulle-allocator")
install( FILES ${PRIVATE_HEADERS} DESTINATION "include/mulle-allocator/private")

install( FILES ${CMAKE_INCLUDES}  DESTINATION "include/mulle-allocator/cmake")

include( PostInstall OPTIONAL)
