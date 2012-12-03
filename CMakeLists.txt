cmake_minimum_required(VERSION 2.8)
project(MGZ_UTILS)

include(FindDoxygen)
include(CheckFunctionExists) 

add_definitions(-Wall)
if(DEBUG)
  set(CMAKE_C_FLAGS "-g -O0 -DDEBUG")
  set(CMAKE_CXX_FLAGS "-g -O0 -DDEBUG")
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

if(CPP11) 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
endif()

set(MGZ_UTILS_OUTPUT_TEMP_DIR ${CMAKE_BINARY_DIR}/.build)
file(MAKE_DIRECTORY ${MGZ_UTILS_OUTPUT_TEMP_DIR})

CHECK_FUNCTION_EXISTS(vasprintf HAVE_VASPRINTF)
if(HAVE_VASPRINTF)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_VASPRINTF")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_VASPRINTF")
endif(HAVE_VASPRINTF)

# Documentation
if(DOXYGEN_FOUND)
  find_program(GRAPHVIZ
    dot
    )
  if("${GRAPHVIZ}" STREQUAL "GRAPHVIZ-NOTFOUND")
    message(STATUS "GraphViz not found!")
    set(HAVE_DOT NO)
  else()
    message(STATUS "Found GraphViz: ${GRAPHVIZ}")
    set(HAVE_DOT YES)
  endif()
  configure_file(
    ${MGZ_UTILS_SOURCE_DIR}/Doxyfile.in 
    ${MGZ_UTILS_OUTPUT_TEMP_DIR}/Doxyfile
    )

  set(DOXYGEN_INPUT ${MGZ_UTILS_OUTPUT_TEMP_DIR}/Doxyfile)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT})
endif()

enable_testing()
add_subdirectory(src)
add_subdirectory(ext)
add_subdirectory(test)
