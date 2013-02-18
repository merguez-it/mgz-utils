project(MGZ_UTILS_DEBUG_LIB)

install(DIRECTORY ${MGZ_UTILS_SOURCE_DIR}/include/debug
  DESTINATION include
  PATTERN "internal" EXCLUDE
  )
