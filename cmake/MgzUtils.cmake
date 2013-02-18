function(set_mgz_version STLIB SHLIB) 
  set_target_properties(${STLIB}
    PROPERTIES
    OUTPUT_NAME "${SHLIB}"
    )
  if(WIN32)
    set_target_properties(${SHLIB}
      PROPERTIES
      PREFIX ""
      IMPORT_SUFFIX ${CMAKE_IMPORT_LIBRARY_SUFFIX}
      )
    set_target_properties(${STLIB}
      PROPERTIES
      PREFIX ""
      SUFFIX .lib
      IMPORT_SUFFIX ${CMAKE_IMPORT_LIBRARY_SUFFIX}
      )
  endif()
  set_target_properties(${SHLIB}
    PROPERTIES 
    SOVERSION ${MGZ_UTILS_VERSION}
    VERSION ${MGZ_UTILS_LVERSION}
    )
endfunction()
