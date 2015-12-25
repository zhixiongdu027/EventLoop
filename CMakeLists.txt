cmake_minimum_required(VERSION 2.8.4)
project(EventLoop)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g  -Wall -W -std=c++11  -O3")

set(EVENTLOOP_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/src/EventLoop/Channel.cpp
        ${PROJECT_SOURCE_DIR}/src/EventLoop/Channel.h
        ${PROJECT_SOURCE_DIR}/src/EventLoop/Forward.h
        ${PROJECT_SOURCE_DIR}/src/EventLoop/EventLoop.h
        ${PROJECT_SOURCE_DIR}/src/EventLoop/EventLoop.cpp
        ${PROJECT_SOURCE_DIR}/src/EventLoop/tool/StreamBuffer.cpp
        ${PROJECT_SOURCE_DIR}/src/EventLoop/tool/StreamBuffer.h
        ${PROJECT_SOURCE_DIR}/src/EventLoop/tool/TaskWheel.h
        ${PROJECT_SOURCE_DIR}/src/EventLoop/tool/SocketHelp.c
        ${PROJECT_SOURCE_DIR}/src/EventLoop/tool/SocketHelp.h
        ${PROJECT_SOURCE_DIR}/src/EventLoop/tool/BlockingQueue.hpp
        ${PROJECT_SOURCE_DIR}/src/EventLoop/tool/ThreadPool.h
        ${PROJECT_SOURCE_DIR}/src/EventLoop/tool/Likely.h
        ${PROJECT_SOURCE_DIR}/src/EventLoop/tool/SimpleEncode.h
        )

add_library(eventloop ${EVENTLOOP_SOURCE_FILES})

include_directories("${PROJECT_SOURCE_DIR}/src")

add_subdirectory("example")