project(MGZ_UTILS_UTIL_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

set(MGZ_UTIL_SOURCES
  getopt.cc
  internal/strptime.c
  datetime.cc
  internal/varg.cc
  string.cc
  )
add_library(mgz-util SHARED ${MGZ_UTIL_SOURCES})
add_library(mgz-util-s STATIC ${MGZ_UTIL_SOURCES})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-util.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-util.pc
  @ONLY
  )
set(MGZ_UTIL_HEADERS 
  ${MGZ_UTILS_SOURCE_DIR}/include/util/any.h
  ${MGZ_UTILS_SOURCE_DIR}/include/util/datetime.h
  ${MGZ_UTILS_SOURCE_DIR}/include/util/exception.h
  ${MGZ_UTILS_SOURCE_DIR}/include/util/map.h
  ${MGZ_UTILS_SOURCE_DIR}/include/util/singleton.h
  ${MGZ_UTILS_SOURCE_DIR}/include/util/string.h
  ${MGZ_UTILS_SOURCE_DIR}/include/util/types.h
  ${MGZ_UTILS_SOURCE_DIR}/include/util/vector.h
  ${MGZ_UTILS_SOURCE_DIR}/include/util/getopt.h
  )
install(FILES ${MGZ_UTIL_HEADERS} DESTINATION include/util)
install(TARGETS mgz-util mgz-util-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-util.pc DESTINATION lib/pkgconfig)
