
set(EXAMPLE_TASK_SOURCE
        HTTP/HTTP.cpp
        HTTP/HTTP.h
        Define.cpp
        Define.h
        main.cpp
        ProxyServer.cpp
        ProxyServer.h)

add_executable(http_proxy ${EXAMPLE_TASK_SOURCE})
target_link_libraries(http_proxy eventloop)