#include <iostream>
#include <msgpack.hpp>
#include "EventLoop/tool/SocketHelp.h"
#include "EventLoop/EventLoop.h"
#include "EventLoop/tool/BlockingQueue.hpp"
#include "EventLoop/tool/ThreadPool.h"

struct A {
    A() : a(std::rand()), b(std::rand()), c(std::rand()) { };
    int a;
    int b;
    int c;
    MSGPACK_DEFINE (a, b, c);
};


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
    BlockData block;
    StreamBuffer *read_buffer = channel_ptr->get_read_buffer();
    while (true) {
        size_t all = 0;
        ExecuteState state = stream_buffer_peek(read_buffer, &all, &type, &block);
        if (state == ExecuteError) {
            loop_ptr->erase_channel(channel_ptr->id());
            return;
        }
        else if (state == ExecuteProcessing) {
            return;
        }
        else if (state == ExecuteDone) {
            msgpack::unpacked unpacked = msgpack::unpack(block.data, block.len);
            std::unique_ptr<Message> ptr(new Message(type, std::move(unpacked)));
            queue.push(std::move(ptr));
            read_buffer->discard(all);
        }
    }
}

int main() {

    EventLoop loop;

    int listen_sock = create_tcp_listen(10000, 1);

    loop.add_channel(listen_sock,
                     true, true, -1,
                     [](
                             EventLoopPtr &loop_ptr, ChannelPtr
                     &channel_ptr,
                             ChannelEvent event
                     ) {
                         if (event != EVENT_IN) {
                             loop_ptr->stop();
                             return;
                         }
                         int client_fd = accept(channel_ptr->fd(), nullptr, nullptr);
                         loop_ptr->
                                 add_channel(client_fd,
                                             true, false, 100, io_cb);
                     });

    ThreadPool pool;
    //send thread;
    pool.emplace_back([]() {
        EventLoop send_loop;
        int socket_fd = tcp_connect("127.0.0.1", 10000);
        ChannelPtr &channel = send_loop.add_channel(socket_fd, true, false, -1,
                                                    [](EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr,
                                                       ChannelEvent) {
                                                        loop_ptr->erase_channel(channel_ptr->id());
                                                    });
        msgpack::sbuffer buffer;
        for (int i = 0; i < 10; ++i) {
            A temp;
            buffer.clear();
            msgpack::pack(buffer, temp);
            channel_send(channel, (uint32_t) i, BlockData(buffer.data(), buffer.size()));
        }
        std::cout << "send thread will exit" << std::endl;
    }
    );

    //handle thread
    pool.emplace_back([]() {
        while (true) {
            std::unique_ptr<Message> ptr;
            if (queue.pop_wait_for(std::chrono::seconds(3), &ptr)) {
                msgpack::object object = ptr->unpacked.get();
                std::cout << "queue: " << "type :" << ptr->type << " obj " << object <<
                std::endl;
            }
            else {
                std::cout << "handle thread will exit" << std::endl;
                break;
            }
        }
    });

    loop.start();

}
