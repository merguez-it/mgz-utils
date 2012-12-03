cmake_minimum_required (VERSION 2.8)
project(MGZ_UTILS_TESTS)

include_directories(
  ${MGZ_UTILS_EXTERN_SOURCE_DIR}/gtest-1.6.0/include
  ${MGZ_UTILS_SOURCE_DIR}/include
  )

set(TESTS_LIBS
  mgz-util
  mgz-net
  mgz-regex
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

