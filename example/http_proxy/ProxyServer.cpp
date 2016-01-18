//
// Created by adugeek on 15-7-9.
//
#include <iostream>
#include <EventLoop/Forward.h>
#include "ProxyServer.h"
#include "EventLoop/tool/SocketHelp.h"

void ProxyServer::proxy_server_forward_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelId events) {
    ChannelPtr &client_ptr = loop_ptr->get_channel(channel_ptr->context.u32);
    if (client_ptr != nullptr && events == EVENT_IN && channel_ptr->read() > 0) {
        loop_ptr->add_channel_lifetime(channel_ptr->id(), 60);
        StreamBuffer *buffer = channel_ptr->get_read_buffer();
        client_ptr->send(buffer->peek(), buffer->peek_able());
        buffer->discard_all();
    }
    else {
        loop_ptr->erase_channel(channel_ptr->context.u32);
        loop_ptr->erase_channel(channel_ptr->id());
    }
}

void ProxyServer::proxy_client_forward_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelId events) {
    ChannelContext *channel_context = static_cast<ChannelContext *>(channel_ptr->context.ptr);
    ChannelPtr &remote_ptr = loop_ptr->get_channel(channel_context->other_side);
    if (remote_ptr != nullptr && events == EVENT_IN && channel_ptr->read() > 0) {
        loop_ptr->add_channel_lifetime(channel_ptr->id(), 60);
        StreamBuffer *buffer = channel_ptr->get_read_buffer();
        remote_ptr->send(buffer->peek(), buffer->peek_able());
        buffer->discard_all();
    }
    else {
        loop_ptr->erase_channel(channel_context->other_side);
        loop_ptr->erase_channel(channel_ptr->id());
    }
}

void ProxyServer::proxy_client_connect_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelId events) {
    if (events == EVENT_IN && channel_ptr->read() > 0) {
        loop_ptr->add_channel_lifetime(channel_ptr->id(), 60);
        do {
            ChannelContext *channel_context = static_cast<ChannelContext *>(channel_ptr->context.ptr);
            StreamBuffer *buffer = channel_ptr->get_read_buffer();
            HTTP::HTTP_STATUS parser_status = channel_context->http_parser.parse(buffer->peek(), buffer->peek_able());
            if (parser_status == HTTP::NEED_MORE) {
                return;
            }
            if (parser_status == HTTP::BAD) {
                break;
            }
            auto &head_info = channel_context->http_parser.head_info;
            if (head_info.find("HOST") == head_info.end()) { break; }
            std::string remote_host = std::string(buffer->peek(head_info["HOST"].first), head_info["HOST"].second);
            std::cout << "remote host :" << remote_host << std::endl;
            if (remote_host == "") { break; }
            int remote_sock;
            if (tcp_nonblock_connect(remote_host.c_str(), 80, &remote_sock) == ExecuteError) { break; };

            ChannelPtr &remote_ptr = loop_ptr->add_connecting_channel(remote_sock, 5, 60, proxy_server_forward_cb);
            if (remote_ptr == nullptr) { break; }
            remote_ptr->send(buffer->peek(), buffer->peek_able());
            buffer->discard_all();
            remote_ptr->context.u32 = channel_ptr->id();
            channel_context->http_parser.clear();
            channel_context->other_side = remote_ptr->id();
            channel_ptr->set_event_cb(proxy_client_forward_cb);
            return;
        } while (false);
    }
    loop_ptr->erase_channel(channel_ptr->id());
}

void ProxyServer::proxy_listen_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelId events) {
    if (events == EVENT_IN) {
        int nfd = accept4(channel_ptr->fd(), nullptr, nullptr, SOCK_NONBLOCK);
        ChannelPtr &new_channel = loop_ptr->add_channel(nfd, true, true, 60, proxy_client_connect_cb);
        if (new_channel == nullptr) {
            close(nfd);
            return;
        }
        ChannelContext *new_context = new ChannelContext;
        new_channel->context.ptr = new_context;
        new_channel->context_deleter = [](void *rhs) { delete static_cast<ChannelContext *>(rhs); };
    }
    else {
        loop_ptr->stop();
    }
}

void ProxyServer::run() {
    EventLoop event_loop;

    if (event_loop.add_channel(proxy_listen_, true, true, -1, proxy_listen_cb) == nullptr) {
        return;
    };
    event_loop.start();
}

