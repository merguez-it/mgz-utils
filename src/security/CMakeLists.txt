project(MGZ_UTILS_SECURITY_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

if(WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/mgz-security.rc.in
    ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-security.rc
    )
  set(MGZ_UTILS_SECURITY_RC ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-security.rc)
endif()

set(MGZ_SECURITY_SOURCES
  adler32.cc
  md5.cc
  sha1.cc
  sha2.cc
  crc32.cc
  ripem.cc
  ${MGZ_UTILS_SECURITY_RC}
  )
add_library(mgz-security SHARED ${MGZ_SECURITY_SOURCES})
target_link_libraries(mgz-security mgz-util)
add_library(mgz-security-s STATIC ${MGZ_SECURITY_SOURCES})
set_mgz_version(mgz-security-s mgz-security)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-security.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-security.pc
  @ONLY
  )

install(DIRECTORY ${MGZ_UTILS_SOURCE_DIR}/include/security
  DESTINATION include
  PATTERN "internal" EXCLUDE
  )
install(TARGETS mgz-security mgz-security-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION lib
  )
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-security.pc DESTINATION lib/pkgconfig)
