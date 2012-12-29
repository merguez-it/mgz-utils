project(MGZ_UTILS_IO_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

set(MGZ_IO_SOURCES
  file.cc
  faststream.cc
  stream.cc
  )
add_library(mgz-io SHARED ${MGZ_IO_SOURCES})
target_link_libraries(mgz-io mgz-util mgz-regex)

add_library(mgz-io-s STATIC ${MGZ_IO_SOURCES})

set(MGZ_IO_HEADERS 
  ${MGZ_UTILS_SOURCE_DIR}/include/io/faststream.h
  ${MGZ_UTILS_SOURCE_DIR}/include/io/file.h
  ${MGZ_UTILS_SOURCE_DIR}/include/io/stream.h
  )
install(FILES ${MGZ_IO_HEADERS} DESTINATION include/io)
install(TARGETS mgz-io mgz-io-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
