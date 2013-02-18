cmake_minimum_required (VERSION 2.8)
project(MGZ_TESTS)

set(MGZ_TEST_PATH ${CMAKE_CURRENT_SOURCE_DIR})
configure_file(
  ${MGZ_TESTS_SOURCE_DIR}/config-test.h.in
  config-test.h
  )

include_directories(
  ${MGZ_UTILS_EXTERN_SOURCE_DIR}/gtest-1.6.0/include
  ${MGZ_UTILS_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(TESTS_LIBS
  mgz-compress-s
  mgz-io-s
  mgz-security-s
  mgz-xml-s
  mgz-regex-s
  mgz-net-s
  mgz-util-s
  gtest
  gtest_main
  )

add_executable(uri_unittest "uri_unittest.cc")
target_link_libraries(uri_unittest ${TESTS_LIBS})
add_test(URI_UNITTEST uri_unittest)

add_executable(mime_unittest "mime_unittest.cc")
target_link_libraries(mime_unittest ${TESTS_LIBS})
add_test(MIME_UNITTEST mime_unittest)

add_executable(datetime_unittest "datetime_unittest.cc")
target_link_libraries(datetime_unittest ${TESTS_LIBS})
add_test(DATETIME_UNITTEST datetime_unittest)

add_executable(re_unittest "re_unittest.cc")
target_link_libraries(re_unittest ${TESTS_LIBS})
add_test(RE_UNITTEST re_unittest)

add_executable(types_unittest "types_unittest.cc")
target_link_libraries(types_unittest ${TESTS_LIBS})
add_test(TYPES_UNITTEST types_unittest)

add_executable(string_unittest "string_unittest.cc")
target_link_libraries(string_unittest ${TESTS_LIBS})
add_test(STRING_UNITTEST string_unittest)

add_executable(exception_unittest "exception_unittest.cc")
target_link_libraries(exception_unittest ${TESTS_LIBS})
add_test(EXCEPTION_UNITTEST exception_unittest)


add_executable(getopt_unittest "getopt_unittest.cc")
target_link_libraries(getopt_unittest ${TEST_LIBS})
add_test(GETOPT_UNITTEST getopt_unittest)

add_executable(file_unittest "file_unittest.cc")
target_link_libraries(file_unittest ${TESTS_LIBS})
add_test(FILE_UNITTEST file_unittest)

add_executable(properties_unittest "properties_unittest.cc")
target_link_libraries(properties_unittest ${TESTS_LIBS})
add_test(PROPERTIES_UNITTEST properties_unittest)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_executable(faststream_unittest "faststream_unittest.cc")
  target_link_libraries(faststream_unittest ${TESTS_LIBS})
  add_test(FASTSTREAM_UNITTEST faststream_unittest)
endif()

add_executable(crc32_unittest "crc32_unittest.cc")
target_link_libraries(crc32_unittest ${TESTS_LIBS})
add_test(CRC32_UNITTEST crc32_unittest)

add_executable(md5_unittest "md5_unittest.cc")
target_link_libraries(md5_unittest ${TESTS_LIBS})
add_test(MD5_UNITTEST md5_unittest)

add_executable(sha1_unittest "sha1_unittest.cc")
target_link_libraries(sha1_unittest ${TESTS_LIBS})
add_test(SHA1_UNITTEST sha1_unittest)

add_executable(sha2_unittest "sha2_unittest.cc")
target_link_libraries(sha2_unittest ${TESTS_LIBS})
add_test(SHA2_UNITTEST sha2_unittest)

add_executable(ripem_unittest "ripem_unittest.cc")
target_link_libraries(ripem_unittest ${TESTS_LIBS})
add_test(RIPEM_UNITTEST ripem_unittest)

add_executable(compress_unittest "compress_unittest.cc")
target_link_libraries(compress_unittest ${TESTS_LIBS})
add_test(COMPRESS_UNITTEST compress_unittest)

add_executable(filesystem_unittest "filesystem_unittest.cc")
target_link_libraries(filesystem_unittest ${TESTS_LIBS})
add_test(FILESYSTEM_UNITTEST filesystem_unittest)

add_executable(zip_unittest "zip_unittest.cc")
target_link_libraries(zip_unittest ${TESTS_LIBS})
add_test(ZIP_UNITTEST zip_unittest)

