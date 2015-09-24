cmake_minimum_required(VERSION 2.8.4)
project(MyProxy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -W -O3")

set(SOURCE_FILES
        src/EventLoop/Channel.cpp
        src/EventLoop/Channel.h
        src/EventLoop/ChannelForward.h
        src/EventLoop/EventLoop.h
        src/EventLoop/EventLoop.cpp
        src/EventLoop/StreamBuffer.cpp
        src/EventLoop/StreamBuffer.h
        src/EventLoop/TimerWheel.cpp
        src/EventLoop/TimerWheel.h
        src/EventLoop/SockHelp/SocketHelp.c
        src/EventLoop/SockHelp/SocketHelp.h
        src/main.cpp
)

add_executable(MyProxy ${SOURCE_FILES})
