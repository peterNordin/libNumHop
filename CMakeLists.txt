cmake_minimum_required(VERSION 3.0)
project(NumHop)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)


file(GLOB_RECURSE srcfiles src/*.cc)

add_library(NumHop STATIC ${srcfiles})
target_include_directories(NumHop PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

install(TARGETS NumHop
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include )

add_subdirectory(test)
