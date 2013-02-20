project(MGZ_UTILS_COMPRESS_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

if(WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/mgz-compress.rc.in
    ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-compress.rc
    )
  set(MGZ_UTILS_COMPRESS_RC ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-compress.rc)
endif()

set(MGZ_COMPRESS_SOURCES
  internal/deflate.c
  internal/inflate.c
  z.cc
  compressor.cc
  compressor/gzip.cc
  compressor/pkzip.cc
  compressor/zlib.cc
  compressor/raw.cc
  archive/internal/common.cc
  archive/unzip.cc
  archive/zip.cc
  ${MGZ_UTILS_COMPRESS_RC}
  )
add_library(mgz-compress SHARED ${MGZ_COMPRESS_SOURCES})
target_link_libraries(mgz-compress mgz-util mgz-security mgz-io)
add_library(mgz-compress-s STATIC ${MGZ_COMPRESS_SOURCES})
set_mgz_version(mgz-compress-s mgz-compress)

# Installation

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-compress.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-compress.pc
  @ONLY
  )
install(DIRECTORY ${MGZ_UTILS_SOURCE_DIR}/include/compress
  DESTINATION include
  PATTERN "internal" EXCLUDE
  )
install(TARGETS mgz-compress mgz-compress-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION lib
  )
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-compress.pc DESTINATION lib/pkgconfig)
