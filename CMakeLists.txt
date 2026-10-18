cmake_minimum_required( VERSION 3.13...99.99)

project( mulle-allocator VERSION 9.0.0 LANGUAGES C)

#
# The library advertises ANSI/ISO C (see the "std" and "apis" badges in the
# README). Compile it as C11 with extensions off, and define no feature test
# macro at all: strict C mode then offers only the standard declarations, so a
# POSIX or BSD call is a compile error instead of passing silently. That is what
# "apis: ansi" claims, so the build checks the claim too.
#
# A standard given on the command line wins, so other dialects remain testable
# with e.g. -DCMAKE_C_STANDARD=23 or CFLAGS=-std=c23.
#
if( NOT CMAKE_C_STANDARD
    AND NOT CMAKE_C_FLAGS MATCHES "(^| )-std="
    AND NOT "$ENV{CFLAGS}" MATCHES "(^| )-std=")
   set( CMAKE_C_STANDARD 11)
   set( CMAKE_C_EXTENSIONS OFF)
endif()


### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/reflect")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include( Environment)

include( Files)

# Configure version header

### Library

if( HEADERS_PHASE)
   install( FILES ${INSTALL_PUBLIC_HEADERS}  DESTINATION "include/mulle-allocator")
   install( FILES ${INSTALL_PRIVATE_HEADERS} DESTINATION "include/mulle-allocator")
   install( FILES ${INSTALL_CMAKE_INCLUDES}  DESTINATION "include/mulle-allocator/cmake")

   # short cut out
   if( NOT COMPILE_PHASE AND NOT LINK_PHASE)
      return()
   endif()
endif()

if( LINK_PHASE)
   include( Dependencies OPTIONAL)
endif()

include( Library)
include( InstallLibrary)

if( LINK_PHASE)
   # we don't need these headers, during compile so fine if we do it during
   # link only
   include( AddSubdirectorySupport)
   include( FindPackageSupport)

#   # can do this multiple times
#   install( TARGETS ${LIBRARY_NAME}
#            EXPORT ${PROJECT_NAME}-targets)
#
#
#   target_include_directories( ${LIBRARY_NAME} PUBLIC
#       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#       $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
#
#   # Set target properties
#   set_target_properties( ${LIBRARY_NAME} PROPERTIES
#                          VERSION ${PROJECT_VERSION}
#                          SOVERSION ${PROJECT_VERSION_MAJOR}
#                          EXPORT_NAME ${PROJECT_NAME})
#
#   # Create and install config files
#   include( CMakePackageConfigHelpers)
#
#   configure_package_config_file(
#       cmake/${PROJECT_NAME}-config.cmake.in
#       ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
#       INSTALL_DESTINATION lib/cmake/${PROJECT_NAME})
#
#   configure_file( cmake/${PROJECT_NAME}-config.h.in ${PROJECT_NAME}-config.h)
#
#   install( EXPORT ${PROJECT_NAME}-targets
#            FILE ${PROJECT_NAME}-targets.cmake
#            NAMESPACE ${PROJECT_NAME}::
#            DESTINATION lib/cmake/${PROJECT_NAME})
#
#   write_basic_package_version_file(
#       "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
#       VERSION ${PROJECT_VERSION}
#       COMPATIBILITY SameMajorVersion)
#
#   install(FILES
#     "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
#     "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
#     DESTINATION lib/cmake/${PROJECT_NAME}
#   )

endif()

include( FinalOutput OPTIONAL)
