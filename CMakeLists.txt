cmake_minimum_required(VERSION 3.14)

option(
  BUILD_SHARED "Build DLL" False
)

if(BUILD_SHARED EQUAL True)
set(BUILD_TYPE SHARED)
else()
set(BUILD_TYPE STATIC)
endif()

project(pnet)

set(SOURCES
  src/net.c
)
add_library(pnet ${BUILD_TYPE} ${SOURCES})
target_include_directories(pnet PUBLIC include/)

if (WIN32)
target_link_libraries(pnet ws2_32)
else()

endif()

add_subdirectory(test)