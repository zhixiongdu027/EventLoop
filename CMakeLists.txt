cmake_minimum_required(VERSION 2.8.4)
project(EventLoop)

#set(CMAKE_CXX_COMPILER /usr/bin/clang)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g  -Wall -W -std=c++11  -O3")

include_directories("${PROJECT_SOURCE_DIR}/src")

add_subdirectory("src/EventLoop")

add_subdirectory("example")