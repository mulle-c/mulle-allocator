if( NOT __MULLE_ALLOCATOR_DEPENDENCIES_AND_LIBRARIES_CMAKE__)
   set( __MULLE_ALLOCATOR_DEPENDENCIES_AND_LIBRARIES_CMAKE__ ON)

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
endif()
