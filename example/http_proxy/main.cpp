//
// Created by adugeek on 15-7-1.
//


#include "ProxyServer.h"
#include "EventLoop/tool/SocketHelp.h"


int main() {
    int proxy_listen_socket = create_tcp_listen(3128, 1);
    if (proxy_listen_socket < 0) {
        return -1;
    }
    ProxyServer server(proxy_listen_socket);
    server.run();
}