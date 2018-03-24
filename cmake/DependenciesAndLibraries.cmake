if( NOT __MULLE_TEST_ALLOCATOR_DEPENDENCIES_AND_LIBRARIES_CMAKE__)
   set( __MULLE_TEST_ALLOCATOR_DEPENDENCIES_AND_LIBRARIES_CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"")
   endif()

   #
   # The following includes include definitions generated
   # during `mulle-sde update`. Don't edit those files. They are
   # overwritten frequently.
   #
   # === MULLE-SDE START ===

   include( _Dependencies)
   include( _Libraries)

   # === MULLE-SDE END ===
   #

   #
   # If you need more find_library() statements, that you dont want to manage
   # with the sourcetree, add them here.
   #
   # Add OS specific dependencies to OS_SPECIFIC_LIBRARIES
   # Add all other dependencies (rest) to DEPENDENCY_LIBRARIES
   #

   #
   # OPTIONAL_DEPENDENCY_LIBRARIES, to be included in standalone libaries
   #
   if( NOT MULLE_TEST_ALLOCATOR_LIBRARY)
      find_library( MULLE_TEST_ALLOCATOR_LIBRARY NAMES mulle-test-allocator)
      message( STATUS "MULLE_TEST_ALLOCATOR_LIBRARY is ${MULLE_TEST_ALLOCATOR_LIBRARY}")
      set( OPTIONAL_DEPENDENCY_LIBRARIES
         ${MULLE_TEST_ALLOCATOR_LIBRARY}
         ${OPTIONAL_DEPENDENCY_LIBRARIES}
         CACHE INTERNAL "need to cache this"
      )
   endif()

endif()
