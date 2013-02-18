project(MGZ_UTILS_IO_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

set(MGZ_IO_SOURCES
  file.cc
  properties.cc
  faststream.cc
  stream.cc
  filesystem.cc
  )
add_library(mgz-io SHARED ${MGZ_IO_SOURCES})
target_link_libraries(mgz-io mgz-util mgz-regex mgz-security mgz-xml)
add_library(mgz-io-s STATIC ${MGZ_IO_SOURCES})
set_mgz_version(mgz-io-s mgz-io)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-io.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-io.pc
  @ONLY
  )

install(DIRECTORY ${MGZ_UTILS_SOURCE_DIR}/include/io
  DESTINATION include
  PATTERN "internal" EXCLUDE
  )
install(TARGETS mgz-io mgz-io-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION lib
  )
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-io.pc DESTINATION lib/pkgconfig)
