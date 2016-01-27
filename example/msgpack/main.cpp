#include <iostream>
#include <msgpack.hpp>
#include "EventLoop/tool/SocketHelp.h"
#include "EventLoop/EventLoop.h"
#include "EventLoop/tool/BlockingQueue.hpp"
#include "EventLoop/tool/ThreadPool.h"

struct Message {

    Message(uint32_t type, msgpack::unpacked &&unpacked) : type(type), unpacked(std::move(unpacked)) { };

    uint32_t type;

    msgpack::unpacked unpacked;
};

BlockingQueue<std::unique_ptr<Message> > queue;

void io_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelEvent event) {
    if (event != EVENT_IN || channel_ptr->read() <= 0) {
        loop_ptr->erase_channel(channel_ptr->id());
        return;
    }

    uint32_t type;
    char *data;
    size_t data_len = 100;

    while (true) {
        ExecuteState state = channel_ptr->peek_block_data(&type, &data, &data_len);
        if (state == ExecuteError) {
            loop_ptr->erase_channel(channel_ptr->id());
            return;
        }
        else if (state == ExecuteProcessing) {
            return;
        }
        else if (state == ExecuteDone) {
            msgpack::unpacked unpacked = msgpack::unpack(data, data_len);
            std::unique_ptr<Message> ptr(new Message(type, std::move(unpacked)));
            queue.push(std::move(ptr));
            channel_ptr->discard_block_data(data_len);
        }
    }
}

int main() {

    EventLoop loop;

    int listen_sock = create_tcp_listen(10000, 1);

    ThreadPool pool;
    pool.emplace_back([]() {
        while (true) {
            std::unique_ptr<Message> ptr = std::move(queue.pop());
            msgpack::object object = ptr->unpacked.get();
            std::cout << "queue: " << "type :" << ptr->type << " obj " << object << std::endl;
        }
    });

    pool.detach_all();

    loop.add_channel(listen_sock, true, true, -1,
                     [](EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelEvent event) {
                         if (event != EVENT_IN) {
                             loop_ptr->stop();
                             return;
                         }
                         int client_fd = accept(channel_ptr->fd(), nullptr, nullptr);
                         loop_ptr->add_channel(client_fd, true, false, 100, io_cb);
                     });
    loop.start();
}
