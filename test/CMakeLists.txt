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
  mgz-util
  mgz-net
  mgz-regex
  mgz-io
  mgz-security
  gtest
  gtest_main
  )

add_executable(uri_unittest "uri_unittest.cc")
target_link_libraries(uri_unittest ${TESTS_LIBS})
add_test(URI_UNITTEST uri_unittest)

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

add_executable(file_unittest "file_unittest.cc")
target_link_libraries(file_unittest ${TESTS_LIBS})
add_test(FILE_UNITTEST file_unittest)

add_executable(faststream_unittest "faststream_unittest.cc")
target_link_libraries(faststream_unittest ${TESTS_LIBS})
add_test(FASTSTREAM_UNITTEST faststream_unittest)

add_executable(crc32_unittest "crc32_unittest.cc")
target_link_libraries(crc32_unittest ${TESTS_LIBS})
add_test(CRC32_UNITTEST crc32_unittest)

add_executable(md5_unittest "md5_unittest.cc")
target_link_libraries(md5_unittest ${TESTS_LIBS})
add_test(MD5_UNITTEST md5_unittest)

add_executable(sha1_unittest "sha1_unittest.cc")
target_link_libraries(sha1_unittest ${TESTS_LIBS})
add_test(SHA1_UNITTEST sha1_unittest)

