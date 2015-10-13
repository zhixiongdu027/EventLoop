// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_CHANNEL_H
#define EVENTLOOP_CHANNEL_H


#include "ChannelForward.h"
#include <unordered_map>
#include <sys/socket.h>
#include <unistd.h>
#include "StreamBuffer.h"
#include "TimerWheel.h"

class Channel {
 friend class EventLoop;

 public:
  void *context;
  std::function<void(void *)> delete_context;

  inline int fd() const noexcept {
    return fd_;
  }

  inline ChannelId id() const noexcept {
    return id_;
  }

  inline int recv() noexcept {
    return readBuffer_.read_fd(fd_, nullptr);
  }

  inline void add_live_time(size_t seconds) noexcept {
    will_add_live_time_ = (int) seconds;
  }

  inline void shutdown() noexcept {
    channel_event_map_[id_] |= TODO_SHUTDOWN;
  }

  inline void erase() noexcept {
    channel_event_map_[id_] |= TODO_ERASE;
  }

  inline StreamBuffer *get_read_buffer() noexcept {
    return &readBuffer_;
  }

  inline void set_event_cb(ChannelCallback &cb) noexcept { event_cb_ = cb; }

  void send(const void *data, size_t len, bool is_noblock_fd) noexcept;

  void send_to_socket(const void *data, size_t len, const sockaddr *addr, socklen_t *addr_len) noexcept;

  ~Channel() noexcept {
    if (context != nullptr && delete_context != nullptr) {
      delete_context(context);
    }
    close(fd_);
  };

 private:
  static inline ChannelId make_channel_id() noexcept {
    static ChannelId id = 1;
    return __sync_fetch_and_add(&id, 1);
  }

  Channel(int fd,
          ssize_t timeout,
          std::unordered_map<ChannelId, ChannelEvent> &event_map)
      : context(nullptr), id_(make_channel_id()), fd_(fd), will_add_live_time_(timeout), connected_(true),
        channel_event_map_(event_map) {
  }

  Channel(const Channel &rhs) = delete;
  Channel(Channel &&rhs) = delete;
  Channel &operator=(const Channel &rhs) = delete;

  const ChannelId id_;
  const int fd_;

  ssize_t will_add_live_time_;

  bool connected_;

  ChannelCallback event_cb_;

  StreamBuffer readBuffer_;
  StreamBuffer writeBuffer_;

  std::unordered_map<ChannelId, ChannelEvent> &channel_event_map_;
};

#endif // EVENTLOOP_CHANNEL_H
