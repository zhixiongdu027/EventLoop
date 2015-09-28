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

  ChannelPtr ptr(new Channel(fd, timeout, channel_event_map_, channel_todo_map_));
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

  ChannelId id=ptr->id();
  channel_map_[id]=std::move(ptr);
  return channel_map_[id];
}

int EventLoop::create_timer_fd() {
  struct itimerspec value;
  value.it_value.tv_sec = 1;
  value.it_value.tv_nsec = 0;
  value.it_interval.tv_sec = 1;
  value.it_interval.tv_nsec = 0;

  timer_ = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  return timerfd_settime(timer_, 0, &value, nullptr) < 0 ? -1 : 0;
}

int EventLoop::add_timer_channel() {
  ChannelPtr timer_channel(new Channel(timer_, -1, channel_event_map_, channel_todo_map_));
  if (timer_channel == NULL) {
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
  ChannelId id=timer_channel->id();
  channel_map_[id]=std::move(timer_channel);
  return 0;
}

void EventLoop::handle_cb() {
  auto iterator = channel_event_map_.begin();
  while (iterator != channel_event_map_.end()) {
    const ChannelId id = iterator->first;
    ChannelEvent &events = iterator->second;
    assert(events != 0);
    ChannelPtr &ptr = channel_map_[id];
    if (ptr == nullptr) {
      channel_map_.erase(id);
      channel_event_map_.erase(iterator++);
      continue;
    }
    ptr->will_add_live_time_ = -1;
    ptr->event_cb_(this, ptr, events);
    events &= ~EVENT_IN;
    events &= ~EVENT_TIMEOVER;
    if (ptr->will_add_live_time_ >= 0) {
      timer_wheel_.regist(ptr->id(), static_cast<size_t>(ptr->will_add_live_time_));
    }
    events == 0 ? channel_event_map_.erase(iterator++) : ++iterator;
  }
}

void EventLoop::handle_todo() {
  auto iterator = channel_todo_map_.begin();
  while (iterator != channel_todo_map_.end()) {
    const ChannelId id = iterator->first;
    ChannelTodo &todo = iterator->second;

    ChannelPtr &ptr = channel_map_[id];
    assert(ptr != nullptr);

    if (todo & TODO_ERASE) {
      channel_event_map_.erase(id);
      channel_map_.erase(id);
      channel_todo_map_.erase(iterator++);
      continue;
    }

    reg_event_.data.u32 = id;
    reg_event_.events = 0;

    if (todo & TODO_REGO) {
      reg_event_.events |= (EPOLLIN | EPOLLOUT);
      todo ^= TODO_REGO;
    }

    if (todo & TODO_OUTPUT) {
      if (!ptr->connected_) {
        int error = 0;
        socklen_t sz = sizeof(int);
        int code = getsockopt(ptr->fd(), SOL_SOCKET, SO_ERROR, (void *) &error, &sz);
        if (code < 0 || error != 0) {
          reg_event_.events = 0;
          channel_event_map_[ptr->id()] |= EVENT_CONNECTERR;
        }
        else {
          ptr->connected_ = true;
        }
      }
      if (ptr->connected_) {
        StreamBuffer::IO_RES res = ptr->writeBuffer_.write_fd(ptr->fd(), NULL);
        if (res == StreamBuffer::OK) {
          assert(ptr->writeBuffer_.empty());
          reg_event_.events = EPOLLIN;
        }
        else if (res == StreamBuffer::AGAIN) {
          reg_event_.events = EPOLLIN | EPOLLOUT;
        }
        else {
          reg_event_.events = 0;
          channel_event_map_[ptr->id()] |= EVENT_SENDERR;
        }
      }
      todo ^= TODO_OUTPUT;
    }

    if (todo & TODO_SHUTDOWN) {
      if (ptr->writeBuffer_.empty()) {
        ::shutdown(ptr->fd(), SHUT_RDWR);
        todo ^= TODO_SHUTDOWN;
      }
    }

    if (reg_event_.events != 0 && epoll_ctl(epoll_, EPOLL_CTL_MOD, ptr->fd(), &reg_event_) < 0) {
      channel_event_map_[id] |= EVENT_EPOLLERR;
    }

    todo == 0 ? channel_todo_map_.erase(iterator++) : ++iterator;
  }
}

void EventLoop::loop() {
  epoll_event events[100];
  while (!quit_) {
    int res = epoll_wait(epoll_, events, 100, -1);
    if (res < 0) {
      quit_ = true;
      break;
    }
    for (int i = 0; i < res; ++i) {
      const ChannelId id = events[i].data.u32;
      if (events[i].events & EPOLLHUP) {
        channel_event_map_[id] |= EVENT_HUP;
      }
      if (events[i].events & EPOLLIN) {
        channel_event_map_[id] |= EVENT_IN;
      }
      if (events[i].events & EPOLLOUT) {
        channel_todo_map_[id] |= TODO_OUTPUT;
      }
    }
    handle_cb();
    handle_todo();
  }
}

EventLoop::~EventLoop() {
  if (context != nullptr && delete_context != nullptr) {
    delete_context(context);
  }
  channel_event_map_.clear();
  channel_todo_map_.clear();
  channel_map_.clear();
  close(epoll_);
}