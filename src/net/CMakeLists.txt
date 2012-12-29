project(MGZ_UTILS_NET_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

set(MGZ_NET_SOURCES
  uri.cc
  mime.cc
  )
add_library(mgz-net SHARED ${MGZ_NET_SOURCES})
target_link_libraries(mgz-net mgz-util)

add_library(mgz-net-s STATIC ${MGZ_NET_SOURCES})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-net.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-net.pc
  @ONLY
  )
set(MGZ_NET_HEADERS 
  ${MGZ_UTILS_SOURCE_DIR}/include/net/uri.h
  ${MGZ_UTILS_SOURCE_DIR}/include/net/mime.h
  )
install(FILES ${MGZ_NET_HEADERS} DESTINATION include/net)
install(TARGETS mgz-net mgz-net-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-net.pc DESTINATION lib/pkgconfig)
