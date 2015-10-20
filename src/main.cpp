#include <iostream>
#include "EventLoop/EventLoop.h"
#include "EventLoop/SockHelp/SocketHelp.h"

static void client_cb(EventLoop *loop, ChannelPtr &ptr, ChannelEvent events) {
  if (events & EVENT_IN)
  {
    if (ptr->read()<= 0)
    {
      ptr->erase();
      return;
    }
    StreamBuffer *buffer = ptr->get_read_buffer();
    ptr->send(buffer->peek(), buffer->readable());
    StreamBuffer *write_buffer=ptr->get_write_buffer();
    write_buffer->append("love u" ,6);
    ptr->send();
  }
  else
  {
    std::cout << "events :" << events << std::endl;
    ptr->erase();
  }
}

static void listen_cb(EventLoop *loop, ChannelPtr &ptr, ChannelEvent events) {
  if (events & EVENT_IN) {
    int nfd = accept(ptr->fd(), nullptr, nullptr);
    loop->add_channel(nfd, 1, false, false, client_cb);
  }
}

int main() {
  int fd_8000 = create_tcp_listen(8000, 1);
  EventLoop loop;
  loop.add_channel(fd_8000, -1, false, false, listen_cb);
  loop.start();
}