//
// Created by adugeek on 15-7-7.
//

#ifndef MYPROXY_PROXYSERVER_H
#define MYPROXY_PROXYSERVER_H

#include "Define.h"
#include "HTTP/HTTP.h"
#include "EventLoop/EventLoop.h"
#include <stdint.h>
#include <string>
#include <unistd.h>

struct ChannelContext {
    ChannelContext() {
        other_side = 0;
    }

    HTTP http_parser;
    ChannelId other_side;
};

class ProxyServer {
private:

    static void proxy_server_forward_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelId events);

    static void proxy_client_forward_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelId events);

    static void proxy_client_connect_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelId events);

    static void proxy_listen_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelId events);

public:
    ProxyServer(int proxy_listen_fd) : proxy_listen_(proxy_listen_fd) { };

    ~ProxyServer() { close(proxy_listen_); }

    void run();

private:
    ProxyServer(const ProxyServer &) = delete;

    ProxyServer &operator=(const ProxyServer &) = delete;

    const int proxy_listen_;
};

#endif //MYPROXY_PROXYSERVER_H
