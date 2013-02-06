project(MGZ_UTILS_XML_LIB)
include_directories(${MGZ_UTILS_SOURCE_DIR}/include)

set(MGZ_XML_SOURCES
  xml.cc
  xmlerror.cc
  xmlparser.cc
  )
add_library(mgz-xml SHARED ${MGZ_XML_SOURCES})
target_link_libraries(mgz-xml mgz-util mgz-regex mgz-security)

add_library(mgz-xml-s STATIC ${MGZ_XML_SOURCES})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-xml.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-xml.pc
  @ONLY
  )
set(MGZ_XML_HEADERS 
  ${MGZ_UTILS_SOURCE_DIR}/include/xml/xml.h
  )

install(FILES ${MGZ_XML_HEADERS} DESTINATION include/xml)
install(TARGETS mgz-xml mgz-xml-s
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-xml.pc DESTINATION lib/pkgconfig)
