cmake_minimum_required(VERSION 2.8.4)
project(MyProxy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g  -Wall -W -std=c++11  -O3")

set(SOURCE_FILES
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
        src/EventLoop/tool/ConcurrentQueue.hpp
        src/EventLoop/tool/NoExitWorkerPool.hpp
        src/main.cpp
        )

add_executable(EventLoop ${SOURCE_FILES})
target_link_libraries(EventLoop pthread jsoncpp)