#include <iostream>
#include "EventLoop/EventLoop.h"
#include "EventLoop/SockHelp/SocketHelp.h"

int main() {
    int listen_fd = create_tcp_listen(9000, 1);
    EventLoop loop;

    ChannelCallback client_cb = [&](EventLoopPtr &loop, ChannelPtr &channel_ptr, ChannelEvent events) {
        if (events & EVENT_IN) {
            if (channel_ptr->read() <= 0) {
                channel_ptr->erase();
            }
            channel_ptr->send("love u", 6);
            channel_ptr->shutdown();
        }
        else {
            std::cout << "events :" << events << std::endl;
            channel_ptr->erase();
        }
    };

    ChannelCallback listen_cb = [&](EventLoopPtr &loop, ChannelPtr &channel_ptr, ChannelEvent events) {
        if (events & EVENT_IN) {
            int fd = ::accept(channel_ptr->fd(), nullptr, nullptr);
            loop->add_channel(fd, true, false, 30, client_cb);
        }
    };

    loop.add_channel(listen_fd, true, false, -1, listen_cb);
    loop.start();
}