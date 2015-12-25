cmake_minimum_required(VERSION 2.8.4)
project(EventLoop)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g  -Wall -W -std=c++11  -O3")

set(EVENTLOOP_SOURCE_FILES
        src/EventLoop/Channel.cpp
        src/EventLoop/Channel.h
        src/EventLoop/Forward.h
        src/EventLoop/EventLoop.h
        src/EventLoop/EventLoop.cpp
        src/EventLoop/tool/StreamBuffer.cpp
        src/EventLoop/tool/StreamBuffer.h
        src/EventLoop/tool/TaskWheel.h
        src/EventLoop/tool/SocketHelp.c
        src/EventLoop/tool/SocketHelp.h
        src/EventLoop/tool/BlockingQueue.hpp
        src/EventLoop/tool/ThreadPool.h
        src/EventLoop/tool/Likely.h
        src/EventLoop/tool/SimpleEncode.h
        )

add_library(eventloop ${EVENTLOOP_SOURCE_FILES})

include_directories("${PROJECT_SOURCE_DIR}/src")

add_subdirectory("example")