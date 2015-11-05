#include <iostream>
#include "EventLoop/EventLoop.h"
#include "EventLoop/SockHelp/SocketHelp.h"

void task_cb(EventLoopPtr &loop, ChannelPtr &channel_ptr, void *user_arg, bool *again) {
    StreamBuffer *read_buffer = channel_ptr->get_read_buffer();
    channel_ptr->send(read_buffer->peek(), read_buffer->peek_able());
    *again = true;
}

void client_cb(EventLoopPtr &loop, ChannelPtr &channel_ptr, ChannelEvent events) {
    if (events == EVENT_IN) {
        if (channel_ptr->read() <= 0) {
            loop->erase_channel(channel_ptr->id());
            return;
        }
        loop->add_channel_lifetime(channel_ptr->id(), 30);
        loop->add_task_on_channel(channel_ptr->id(), 3, nullptr, task_cb);
    }
    else {
        std::cout << "events :" << events << std::endl;
        loop->erase_channel(channel_ptr->id());
    }
};

void listen_cb(EventLoopPtr &loop, ChannelPtr &channel_ptr, ChannelEvent events) {
    if (events == EVENT_IN) {
        int fd = ::accept(channel_ptr->fd(), nullptr, nullptr);
        loop->add_channel(fd, true, false, 30, client_cb);
    }
};

int main() {
    int listen_fd = create_tcp_listen(10000, 1);
    EventLoop loop;
    loop.add_channel(listen_fd, true, false, -1, listen_cb);
    loop.start();
}