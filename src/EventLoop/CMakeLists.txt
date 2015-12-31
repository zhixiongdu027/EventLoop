

set(EVENTLOOP_SOURCE_FILES
        Channel.cpp
        Channel.h
        Forward.h
        EventLoop.h
        EventLoop.cpp
        tool/Copyable.h
        tool/StreamBuffer.cpp
        tool/StreamBuffer.h
        tool/TaskWheel.h
        tool/SocketHelp.c
        tool/SocketHelp.h
        tool/BlockingQueue.hpp
        tool/ThreadPool.h
        tool/Likely.h
        tool/SimpleEncode.h
        tool/Singleton.hpp
        )
add_library(eventloop ${EVENTLOOP_SOURCE_FILES})
