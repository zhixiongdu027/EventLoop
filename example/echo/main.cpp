#include <cstdio>
#include "EventLoop/EventLoop.h"
#include "EventLoop/tool/SocketHelp.h"

void channel_task(EventLoopPtr & /*unused*/, ChannelPtr &channel_ptr, void * /*unused*/, bool *again) {
    StreamBuffer *buffer = channel_ptr->get_read_buffer();
    channel_ptr->send(buffer->peek(), buffer->peek_able());
    *again = true;
}

void client_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelEvent events) {
    if (events != EVENT_IN || channel_ptr->read() <= 0) {
        loop_ptr->erase_channel(channel_ptr->id());
        return;
    }

    StreamBuffer *buffer = channel_ptr->get_read_buffer();
    printf("I read :\n%.*s\n", buffer->peek_able(), buffer->peek());
    printf("I will echo to client every 5 seconds\n");

    if (channel_ptr->context.u32 == 0) {
        loop_ptr->add_task_on_channel(true, channel_ptr->id(), 5, nullptr, channel_task);
        channel_ptr->context.u32 = 1;
    }
}

void listen_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelEvent events) {
    if (events != EVENT_IN) {
        loop_ptr->erase_channel(channel_ptr->id());
    }
    int new_fd = accept(channel_ptr->fd(), nullptr, nullptr);
    loop_ptr->add_channel(new_fd, true, false, 30, client_cb);
}

int main() {
    int listen_fd = create_tcp_listen(10000, 1);
    EventLoop loop;
    loop.add_channel(listen_fd, true, true, -1, listen_cb);
    loop.start();
}