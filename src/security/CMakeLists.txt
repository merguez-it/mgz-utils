project(MGZ_UTILS_SECURITY_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

set(MGZ_SECURITY_SOURCES
  md5.cc
  sha1.cc
  crc32.cc
  )
add_library(mgz-security SHARED ${MGZ_SECURITY_SOURCES})
target_link_libraries(mgz-security mgz-util)
add_library(mgz-security-s STATIC ${MGZ_SECURITY_SOURCES})

set(MGZ_SECURITY_HEADERS 
  ${MGZ_UTILS_SOURCE_DIR}/include/security/md5.h
  ${MGZ_UTILS_SOURCE_DIR}/include/security/sha1.h
  ${MGZ_UTILS_SOURCE_DIR}/include/security/crc32.h
  )
install(FILES ${MGZ_SECURITY_HEADERS} DESTINATION include/security)
install(TARGETS mgz-security mgz-security-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
