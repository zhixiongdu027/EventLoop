#include <iostream>
#include "EventLoop/EventLoop.h"
#include "EventLoop/SockHelp/SocketHelp.h"

static void client_cb(EventLoop *loop, ChannelPtr &channel_ptr, ChannelEvent events) {
  if (events & EVENT_IN)
  {
    if (channel_ptr->read()<= 0)
    {
      channel_ptr->erase();
      return;
    }
    StreamBuffer *read_buffer = channel_ptr->get_read_buffer();
    channel_ptr->send(read_buffer->peek(), read_buffer->peek_able());
    channel_ptr->shutdown();
  }
  else
  {
    std::cout << "events :" << events << std::endl;
    channel_ptr->erase();
  }
}

static void listen_cb(EventLoop *loop, ChannelPtr &channel_ptr, ChannelEvent events) {
  if (events & EVENT_IN) {
    int nfd = ::accept(channel_ptr->fd(), nullptr, nullptr);
    loop->add_channel(nfd, true, false, 3, client_cb);
  }
}

int main() {
  int fd_8000 = create_tcp_listen(8000, 1);
  EventLoop loop;
  loop.add_channel(fd_8000, true, false, -1, listen_cb);
  loop.start();
}