project(MGZ_UTILS_NET_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

if(WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/mgz-net.rc.in
    ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-net.rc
    )
  set(MGZ_UTILS_NET_RC ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-net.rc)
endif()

set(MGZ_NET_SOURCES
  uri.cc
  mime.cc
  ${MGZ_UTILS_NET_RC}
  )
add_library(mgz-net SHARED ${MGZ_NET_SOURCES})
target_link_libraries(mgz-net mgz-util)
add_library(mgz-net-s STATIC ${MGZ_NET_SOURCES})
set_mgz_version(mgz-net-s mgz-net)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-net.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-net.pc
  @ONLY
  )

install(DIRECTORY ${MGZ_UTILS_SOURCE_DIR}/include/net
  DESTINATION include
  PATTERN "internal" EXCLUDE
  )
install(TARGETS mgz-net mgz-net-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION lib
  )
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-net.pc DESTINATION lib/pkgconfig)
