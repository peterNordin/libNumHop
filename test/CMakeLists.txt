cmake_minimum_required(VERSION 3.0)
project(numhoptest)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_DEBUG_POSTFIX _d)

file(GLOB_RECURSE srcfiles *.cc)

add_executable(numhoptest ${srcfiles})

target_link_libraries(numhoptest NumHop)

install(TARGETS numhoptest
  RUNTIME DESTINATION bin
)
