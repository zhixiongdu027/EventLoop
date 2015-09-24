// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TIMERWHEEL_H
#define EVENTLOOP_TIMERWHEEL_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "ChannelForward"

class TimerWheel {
 private:
  typedef std::unordered_set<ChannelId> Bucket;

 public:
  explicit TimerWheel(size_t bucket_size) : bucket_vec_(bucket_size + 1), current_index_(0) {
  }

  void regist(ChannelId id, size_t ticks);

  void tick(std::unordered_map<ChannelId, ChannelEvent> &channel_event_map);

  inline void cancel(ChannelId id)
  {
    times_map_.erase(id);
  }

 private:
  TimerWheel(const TimerWheel &) = delete;
  TimerWheel &operator=(const TimerWheel &) = delete;
  std::vector<Bucket> bucket_vec_;
  std::unordered_map<ChannelId, size_t> times_map_;
  size_t current_index_;
};

#endif // EVENTLOOP_WHEEL_H