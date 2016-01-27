
set(EXAMPLE_TASK_SOURCE
        HTTP/HTTP.cpp
        HTTP/HTTP.h
        Define.cpp
        Define.h
        main.cpp
        ProxyServer.cpp
        ProxyServer.h)

add_executable(example_http_proxy ${EXAMPLE_TASK_SOURCE})
target_link_libraries(example_http_proxy eventloop)