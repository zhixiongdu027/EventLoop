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
  std::function<void(void *context)> delete_context;

  ~Channel() {
    if (context != nullptr && delete_context != nullptr) {
      delete_context(context);
    }
    close(fd_);
  };

  inline int fd() const {
    return fd_;
  }

  inline ChannelId id() const {
    return id_;
  }

  inline int recv() {
    return readBuffer_.read_fd(fd_, NULL);
  }

  inline void add_live_time(size_t seconds) {
    will_add_live_time_ = (int) seconds;
  }

  inline void shutdown() {
    channel_todo_map_[id_] |= TODO_SHUTDOWN;
  }

  inline void erase() {
    channel_todo_map_[id_] |= TODO_ERASE;
  }

  inline StreamBuffer *get_read_buffer() {
    return &readBuffer_;
  }

  inline void set_event_cb(ChannelCallback &cb) { event_cb_ = cb; }

  void send(const void *data, size_t len, bool is_noblock_fd);

  void send_to_socket(const void *data, size_t len, const sockaddr *addr, socklen_t *addr_len);

 private:
  static inline ChannelId make_channel_id() {
    static ChannelId id = 1;
    return __sync_fetch_and_add(&id, 1);
  }

  Channel(int fd,
          ssize_t timeout,
          std::unordered_map<ChannelId, ChannelEvent> &event_map,
          std::unordered_map<ChannelId, ChannelTodo> &todo_map)
      : context(nullptr), id_(make_channel_id()), fd_(fd), will_add_live_time_(timeout), connected_(true),
        channel_event_map_(event_map), channel_todo_map_(todo_map) {
  }
  const ChannelId id_;
  const int fd_;

  ssize_t will_add_live_time_;

  bool connected_;

  ChannelCallback event_cb_;

  StreamBuffer readBuffer_;
  StreamBuffer writeBuffer_;

  std::unordered_map<ChannelId, ChannelEvent> &channel_event_map_;
  std::unordered_map<ChannelId, ChannelTodo> &channel_todo_map_;
};

#endif // EVENTLOOP_CHANNEL_H
