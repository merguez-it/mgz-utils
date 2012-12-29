project(MGZ_UTILS_REGEX_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

set(MGZ_REGEX_SOURCES
  re.cc
  trex.c
  )
add_library(mgz-regex SHARED ${MGZ_REGEX_SOURCES})
target_link_libraries(mgz-regex mgz-util)
add_library(mgz-regex-s STATIC ${MGZ_REGEX_SOURCES})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-regex.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-regex.pc
  @ONLY
  )
set(MGZ_REGEX_HEADERS 
  ${MGZ_UTILS_SOURCE_DIR}/include/regex/re.h
  ${MGZ_UTILS_SOURCE_DIR}/include/regex/trex.h
  )
install(FILES ${MGZ_REGEX_HEADERS} DESTINATION include/regex)
install(TARGETS mgz-regex mgz-regex-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-regex.pc DESTINATION lib/pkgconfig)
