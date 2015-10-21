// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_EVENTLOOP_H
#define EVENTLOOP_EVENTLOOP_H

#include <sys/epoll.h>
#include <unordered_map>
#include "ChannelForward.h"
#include "Channel.h"
#include "TimerWheel.h"

class EventLoop {
 public:

  EventLoop() : context(nullptr), init_status_(INIT), epoll_(-1), timer_(-1), quit_(true), timer_wheel_(300) {
  }

  ~EventLoop() noexcept;

  ChannelPtr &add_channel(int fd, bool is_socket, bool is_nonblock, ssize_t live_time, ChannelCallback io_event_cb);

  inline void start() noexcept {
    loop();
  }

  inline void stop() noexcept {
    quit_ = true;
  }

  inline ChannelPtr &get_channel(const ChannelId id) noexcept {
    if (channel_map_.find(id) != channel_map_.end()) {
      return channel_map_[id];
    }
    return null_channel_ptr;
  }

 private:
  EventLoop(const EventLoop &) = delete;
  EventLoop(EventLoop &&rhs) = delete;
  EventLoop &operator=(const EventLoop &) = delete;

  enum INIT_STATUS {
    INIT,
    SUCCESS,
    ERROR
  };

  inline void init() {
    if (create_epoll_fd() < 0 || create_timer_fd() < 0 || add_timer_channel() < 0) {
      init_status_ = ERROR;
    }
    else {
      init_status_ = SUCCESS;
      quit_ = false;
    }
  }

  void loop() noexcept;

  void handle_cb() noexcept;

  inline int create_epoll_fd() noexcept {
    return epoll_ = epoll_create(1);
  }

  int create_timer_fd() noexcept;

  int add_timer_channel();

  inline bool unlawful_fd(int fd) const noexcept {
    return (fd < 0 || fd == epoll_ || fd == timer_);
  }

 public:
  void *context;
  std::function<void(void *)> delete_context;
 private:
  INIT_STATUS init_status_;
  int epoll_;
  int timer_;
  bool quit_;
  std::unordered_map<ChannelId, ChannelPtr> channel_map_;
  std::unordered_map<ChannelId, ChannelEvent> channel_event_map_;
  TimerWheel timer_wheel_;
  epoll_event reg_event_;
  static ChannelPtr null_channel_ptr;
};

#endif // EVENTLOOP_EVENTLOOP_H
