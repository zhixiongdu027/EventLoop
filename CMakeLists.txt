cmake_minimum_required(VERSION 2.8.4)
project(MyProxy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g  -Wall -W -std=c++11  -O3")

set(SOURCE_FILES
        src/EventLoop/Channel.cpp
        src/EventLoop/Channel.h
        src/EventLoop/Forward.h
        src/EventLoop/EventLoop.h
        src/EventLoop/EventLoop.cpp
        src/EventLoop/StreamBuffer.cpp
        src/EventLoop/StreamBuffer.h
        src/EventLoop/TaskWheel.h
        src/EventLoop/SocketHelp.c
        src/EventLoop/SocketHelp.h
        src/main.cpp
        )

add_executable(EventLoop ${SOURCE_FILES})
