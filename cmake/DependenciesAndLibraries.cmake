if( NOT __MULLE_ALLOCATOR_DEPENDENCIES_AND_LIBRARIES_CMAKE__)
   set( __MULLE_ALLOCATOR_DEPENDENCIES_AND_LIBRARIES_CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"")
   endif()

   #
   # Put your find_library() statements here to import other libraries
   #
   # Add OS specific dependencies to OS_SPECIFIC_LIBRARIES
   # Add all other dependencies (rest) to DEPENDENCIES_LIBRARIES


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
   # For benefit of Windows
   #
   #if( MSVC)
   #   set( MULLE_ALLOCATOR_DEFINITIONS $<|PARENT_IDENTIFIER|>_DEFINITIONS})
   #endif()
endif()
