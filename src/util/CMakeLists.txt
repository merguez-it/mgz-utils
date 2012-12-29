project(MGZ_UTILS_UTIL_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

set(MGZ_UTIL_SOURCES
  internal/strptime.c
  datetime.cc
  internal/varg.cc
  string.cc
  )
add_library(mgz-util SHARED ${MGZ_UTIL_SOURCES})
add_library(mgz-util-s STATIC ${MGZ_UTIL_SOURCES})

set(MGZ_UTIL_HEADERS 
  ${MGZ_UTILS_SOURCE_DIR}/include/util/datetime.h
  ${MGZ_UTILS_SOURCE_DIR}/include/util/string.h
  )
install(FILES ${MGZ_UTIL_HEADERS} DESTINATION include/util)
install(TARGETS mgz-util mgz-util-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
