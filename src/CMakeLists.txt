include_directories(${MGZ_UTILS_OUTPUT_TEMP_DIR})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mgz-all.pc.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-all.pc
  @ONLY
  )
install(FILES ${MGZ_UTILS_OUTPUT_TEMP_DIR}/mgz-all.pc DESTINATION lib/pkgconfig)

add_subdirectory(util)
add_subdirectory(net)
add_subdirectory(regex)
add_subdirectory(io)
add_subdirectory(security)
add_subdirectory(xml)
add_subdirectory(compress)
add_subdirectory(debug)

