// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#include "EventLoop.h"
#include <sys/timerfd.h>

ChannelPtr EventLoop::null_channel_ptr = ChannelPtr();

ChannelPtr &EventLoop::add_channel(int fd, ssize_t timeout, ChannelCallback io_event_cb) {
  if (init_status_ == INIT) {
    init();
  }

  if (init_status_ != SUCCESS || unlawful_fd(fd)) {
    return null_channel_ptr;
  }

  ChannelPtr ptr(new Channel(fd, timeout, channel_event_map_));
  if (ptr == nullptr) {
    return null_channel_ptr;
  }

  ptr->set_event_cb(io_event_cb);
  reg_event_.events = EPOLLIN;
  reg_event_.data.u32 = ptr->id();

  if (epoll_ctl(epoll_, EPOLL_CTL_ADD, fd, &reg_event_) < 0) {
    return null_channel_ptr;
  }

  if (ptr->will_add_live_time_ >= 0) {
    timer_wheel_.regist(ptr->id(), static_cast<size_t>(ptr->will_add_live_time_));
  }

  ChannelId id = ptr->id();
  channel_map_[id] = std::move(ptr);
  return channel_map_[id];
}

int EventLoop::create_timer_fd() noexcept {
  struct itimerspec value;
  value.it_value.tv_sec = 1;
  value.it_value.tv_nsec = 0;
  value.it_interval.tv_sec = 1;
  value.it_interval.tv_nsec = 0;

  timer_ = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  return timerfd_settime(timer_, 0, &value, nullptr) < 0 ? -1 : 0;
}

int EventLoop::add_timer_channel() {
  ChannelPtr timer_channel(new Channel(timer_, -1, channel_event_map_));
  if (timer_channel == nullptr) {
    return -1;
  }

  timer_channel->event_cb_ = [this](EventLoop *loop, const ChannelPtr &channel_ptr, ChannelEvent event) {
    uint64_t times;
    if (event & EVENT_IN) {
      if (read(channel_ptr->fd(), &times, sizeof(uint64_t)) != sizeof(uint64_t)) {
        loop->stop();
      }
      timer_wheel_.tick(channel_ptr->channel_event_map_);
      return;
    }
    loop->stop();
  };

  reg_event_.data.u32 = timer_channel->id();
  reg_event_.events = EPOLLIN;

  if (epoll_ctl(epoll_, EPOLL_CTL_ADD, timer_channel->fd(), &reg_event_) < 0) {
    return -1;
  }
  ChannelId id = timer_channel->id();
  channel_map_[id] = std::move(timer_channel);
  return 0;
}

void EventLoop::handle_cb() noexcept {
  auto iterator = channel_event_map_.begin();
  while (iterator != channel_event_map_.end()) {
    const ChannelId channel_id = iterator->first;
    ChannelPtr &channel_ptr = channel_map_[channel_id];

    if (channel_ptr == nullptr) {
      channel_map_.erase(channel_id);
      channel_event_map_.erase(iterator++);
      continue;
    }

    ChannelEvent &channel_event = iterator->second;
    assert(channel_event != 0);

    const uint32_t io_event = channel_event & 0x0000ffff;
    channel_ptr->will_add_live_time_ = -1;
    channel_ptr->event_cb_(this, channel_ptr, io_event);
    if (channel_ptr->will_add_live_time_ >= 0) {
      timer_wheel_.regist(channel_ptr->id(), static_cast<size_t>(channel_ptr->will_add_live_time_));
    }

    channel_event ^= io_event;

    reg_event_.data.u32 = channel_id;
    reg_event_.events = 0;

    if (channel_event & TODO_REGO) {
      reg_event_.events |= (EPOLLIN | EPOLLOUT);
      channel_event ^= TODO_REGO;
    }

    if (channel_event & TODO_OUTPUT) {
      if (!channel_ptr->connected_) {
        int error = 0;
        socklen_t sz = sizeof(int);
        int code = getsockopt(channel_ptr->fd(), SOL_SOCKET, SO_ERROR, (void *) &error, &sz);
        if (code < 0 || error != 0) {
          reg_event_.events = 0;
          channel_event_map_[channel_ptr->id()] |= EVENT_CONNECTERR;
        }
        else {
          channel_ptr->connected_ = true;
        }
      }
      if (channel_ptr->connected_) {
        StreamBuffer::IO_RES res = channel_ptr->writeBuffer_.write_fd(channel_ptr->fd(), nullptr);
        if (res == StreamBuffer::OK) {
          assert(channel_ptr->writeBuffer_.empty());
          reg_event_.events = EPOLLIN;
        }
        else if (res == StreamBuffer::AGAIN) {
          reg_event_.events = EPOLLIN | EPOLLOUT;
        }
        else {
          reg_event_.events = 0;
          channel_event_map_[channel_ptr->id()] |= EVENT_SENDERR;
        }
      }
      channel_event ^= TODO_OUTPUT;
    }
    if (channel_event & TODO_SHUTDOWN) {
      if (channel_ptr->writeBuffer_.empty()) {
        ::shutdown(channel_ptr->fd(), SHUT_RDWR);
        channel_event ^= TODO_SHUTDOWN;
      }
    }
    if (reg_event_.events != 0 && epoll_ctl(epoll_, EPOLL_CTL_MOD, channel_ptr->fd(), &reg_event_) < 0) {
      channel_event_map_[channel_id] |= EVENT_EPOLLERR;
    }
    channel_event == 0 ? channel_event_map_.erase(iterator++) : ++iterator;
  }
}

void EventLoop::loop() noexcept {
  epoll_event events[100];
  while (!quit_) {
    int res = epoll_wait(epoll_, events, 100, -1);
    if (res < 0) {
      quit_ = true;
      break;
    }
    for (int i = 0; i < res; ++i) {
      const ChannelId id = events[i].data.u32;
      uint32_t epoll_event = events[i].events;
      ChannelEvent &channel_event = channel_event_map_[id];
      if (epoll_event & EPOLLHUP) {
        channel_event |= EVENT_HUP;
      }
      if (epoll_event & EPOLLIN) {
        channel_event |= EVENT_IN;
      }
      if (epoll_event & EPOLLOUT) {
        channel_event |= TODO_OUTPUT;
      }
    }
    handle_cb();
  }
}

EventLoop::~EventLoop() noexcept {
  if (context != nullptr && delete_context != nullptr) {
    delete_context(context);
  }
  channel_event_map_.clear();
  channel_map_.clear();
  close(epoll_);
}