cmake_minimum_required(VERSION 2.8)
project(MGZ_UTILS)

include(FindDoxygen)
include(CheckFunctionExists) 
include(CheckSymbolExists)
include(CheckIncludeFiles)
include(CheckIncludeFileCXX)
include(CheckCSourceCompiles)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

add_definitions(-Wall)
if(DEBUG)
  set(CMAKE_C_FLAGS "-g -O0 -DDEBUG")
  set(CMAKE_CXX_FLAGS "-g -O0 -DDEBUG")
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WIN32__")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -D__WIN32__")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__APPLE__")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -D__APPLE__")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__UNIX__")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -D__UNIX__")
endif()

if(CPP11) 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
endif()

set(MGZ_UTILS_OUTPUT_TEMP_DIR ${CMAKE_BINARY_DIR}/.build)
file(MAKE_DIRECTORY ${MGZ_UTILS_OUTPUT_TEMP_DIR})

CHECK_FUNCTION_EXISTS(strptime HAVE_STRPTIME)
CHECK_FUNCTION_EXISTS(vasprintf HAVE_VASPRINTF)
CHECK_FUNCTION_EXISTS(vsnprintf HAVE_VSNPRINTF)
CHECK_INCLUDE_FILES(locale.h HAVE_LOCALE_H)
CHECK_INCLUDE_FILES(windows.h HAVE_WINDOWS_H)
CHECK_INCLUDE_FILES(sys/mman.h HAVE_SYS_MMAN_H)
CHECK_C_SOURCE_COMPILES("#include <unistd.h>
int main(void) {
sysconf(_SC_PAGESIZE);
return 0;
}" HAVE__SC_PAGESIZE) 
CHECK_SYMBOL_EXISTS(MapViewOfFile windows.h HAVE_MAPVIEWOFFILE)
CHECK_SYMBOL_EXISTS(CreateFileMapping windows.h HAVE_CREATEFILEMAPPING)
CHECK_SYMBOL_EXISTS(UnmapViewOfFile windows.h HAVE_UNMAPVIEWOFFILE)
CHECK_SYMBOL_EXISTS(IsWow64Process windows.h HAVE_ISWOW64PROCESS)
CHECK_SYMBOL_EXISTS(GetSystemInfo windows.h HAVE_GETSYSTEMINFO)
CHECK_SYMBOL_EXISTS(GetNativeSystemInfo windows.h HAVE_GETNATIVESYSTEMINFO)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${MGZ_UTILS_OUTPUT_TEMP_DIR}/config.h
  )

CHECK_FUNCTION_EXISTS(pthread_create HAVE_PTHREAD)
CHECK_C_COMPILER_FLAG(-pthread C_HAS_PTHREAD)
CHECK_CXX_COMPILER_FLAG(-pthread CXX_HAS_PTHREAD)

if(C_HAS_PTHREAD)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
elseif(CXX_HAS_PTHREAD)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
elseif(HAVE_PTHREAD)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread")
endif()

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
