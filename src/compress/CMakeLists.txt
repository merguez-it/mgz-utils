project(MGZ_UTILS_COMPRESS_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

set(MGZ_COMPRESS_SOURCES
  internal/deflate.c
  internal/inflate.c
  z.cc
  )
add_library(mgz-compress SHARED ${MGZ_COMPRESS_SOURCES})
add_library(mgz-compress-s STATIC ${MGZ_COMPRESS_SOURCES})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-compress.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-compress.pc
  @ONLY
  )
set(MGZ_COMPRESS_HEADERS 
  ${MGZ_UTILS_SOURCE_DIR}/include/compress/z.h
  )
install(FILES ${MGZ_COMPRESS_HEADERS} DESTINATION include/compress)
install(TARGETS mgz-compress mgz-compress-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-compress.pc DESTINATION lib/pkgconfig)
