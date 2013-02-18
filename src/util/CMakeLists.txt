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
set_mgz_version(mgz-util-s mgz-util)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-util.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-util.pc
  @ONLY
  )

install(DIRECTORY ${MGZ_UTILS_SOURCE_DIR}/include/util
  DESTINATION include
  PATTERN "internal" EXCLUDE
  )
install(TARGETS mgz-util mgz-util-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION lib
  )
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-util.pc DESTINATION lib/pkgconfig)
