

set(EVENTLOOP_SOURCE_FILES
        tool/BlockingQueue.hpp
        tool/Copyable.h
        tool/ExecuteState.h
        tool/Likely.h
        tool/SimpleEncode.h
        tool/Singleton.hpp
        tool/SocketHelp.c
        tool/SocketHelp.h
        tool/StreamBuffer.cpp
        tool/StreamBuffer.h
        tool/TaskWheel.h
        tool/ThreadPool.h
        Channel.cpp
        Channel.h
        EventLoop.h
        EventLoop.cpp
        Forward.h
        )
add_library(eventloop ${EVENTLOOP_SOURCE_FILES})
