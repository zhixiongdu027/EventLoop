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
    ptr->send(buffer->peek(), buffer->readable(), false);
    ptr->add_live_time(3);
  }
  else if (events & EVENT_TIMEOVER) {
    ptr->erase();
  }
}

static void listen_cb(EventLoop *loop, ChannelPtr &ptr, ChannelEvent events) {
  if (events & EVENT_IN) {
    int nfd = accept(ptr->fd(), nullptr, nullptr);
    loop->add_channel(nfd, 1, client_cb);
  }
}

int main() {
  int fd_8000 = create_tcp_listen(8000, 1);
  int fd_9000=  create_tcp_listen(9000,1);
  EventLoop loop;
  loop.add_channel(fd_8000, -1, listen_cb);
  loop.add_channel(fd_9000, -1, listen_cb);
  loop.start();
}