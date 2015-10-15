#include <iostream>
#include "EventLoop/EventLoop.h"
#include "EventLoop/SockHelp/SocketHelp.h"

static void client_cb(EventLoop *loop, ChannelPtr &ptr, ChannelEvent events) {
  if (events & EVENT_IN) {
    if (ptr->recv() < 0) {
      ptr->erase();
      return;
    }
    StreamBuffer *buffer = ptr->get_read_buffer();
    ptr->send_to_socket(buffer->peek(), buffer->readable(), nullptr, nullptr);
    ptr->shutdown();
  }
}

static void listen_cb(EventLoop *loop, ChannelPtr &ptr, ChannelEvent events) {
  if (events & EVENT_IN) {
    int nfd = accept(ptr->fd(), nullptr, nullptr);
    loop->add_channel(nfd, 1, client_cb);
  }
}

int main() {

  StreamBuffer test(10);
  test.append("love",4);

  StreamBuffer test_1;
  std::swap(test_1 ,test);
  StreamBuffer test_2=std::move(test_1);

  write(1 ,test_2.peek() ,test_2.readable());


  int fd_8000 = create_tcp_listen(8000, 1);
  int fd_9000=  create_tcp_listen(9000,1);
  EventLoop loop;
  loop.add_channel(fd_8000, -1, listen_cb);
  loop.add_channel(fd_9000, -1, listen_cb);
  loop.start();
}