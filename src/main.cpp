#include <iostream>
#include "EventLoop/EventLoop.h"
#include "EventLoop/SockHelp/SocketHelp.h"

int main() {
    int myfd;
    if (tcp_nonblock_connect("www.qq.com", 80, &myfd) != NONBLOCK_CONNECT_ERROR) {
        EventLoop loop;
        loop.add_connecting_channel(myfd, -1, 50,
                                    [](EventLoopPtr &loop_ptr, ChannelPtr &channel_ptr, ChannelEvent event) {
                                        std::cout << __LINE__ << std::endl;
                                        std::cout << "EVENTS :" << event << std::endl;
                                        if (event == EVENT_IN) {
                                            if (channel_ptr->read() <= 0) {
                                                std::cout << __LINE__ << std::endl;
                                                loop_ptr->erase_channel(channel_ptr->id());
                                            }
                                            else {
                                                std::cout << __LINE__ << std::endl;
                                                write(1, channel_ptr->get_read_buffer()->peek(),
                                                      channel_ptr->get_read_buffer()->peek_able());
                                            }
                                        }
                                        else {
                                            std::cout << __LINE__ << std::endl;
                                            loop_ptr->erase_channel(channel_ptr->id());
                                        }
                                    })->send("GET /\r\n\r\n", sizeof("GET /\r\n\r\n"));
        loop.start();
    }
}