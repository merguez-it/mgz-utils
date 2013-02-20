project(MGZ_UTILS_XML_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

if(WIN32)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/mgz-xml.rc.in
    ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-xml.rc
    )
  set(MGZ_UTILS_XML_RC ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-xml.rc)
endif()

set(MGZ_XML_SOURCES
  xml.cc
  xmlerror.cc
  xmlparser.cc
  ${MGZ_UTILS_XML_RC}
  )
add_library(mgz-xml SHARED ${MGZ_XML_SOURCES})
add_library(mgz-xml-s STATIC ${MGZ_XML_SOURCES})
set_mgz_version(mgz-xml-s mgz-xml)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-xml.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-xml.pc
  @ONLY
  )

install(DIRECTORY ${MGZ_UTILS_SOURCE_DIR}/include/xml
  DESTINATION include
  PATTERN "internal" EXCLUDE
  )
install(TARGETS mgz-xml mgz-xml-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION lib
  )
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-xml.pc DESTINATION lib/pkgconfig)
