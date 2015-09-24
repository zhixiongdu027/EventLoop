//
// Created by adugeek on 15-7-7.
//
#include  <assert.h>
#include "TimerWheel.h"

void TimerWheel::regist(ChannelId id, size_t ticks) {
  size_t ticks_ = ticks > bucket_vec_.size() - 1 ? bucket_vec_.size() - 1 : ticks;
  Bucket &bucket = bucket_vec_[(current_index_ + ticks_) % bucket_vec_.size()];
  if (bucket.find(id) == bucket.end()) {
    bucket.insert(id);
    times_map_[id] += 1;
  }
}

void TimerWheel::tick(std::unordered_map<ChannelId , ChannelEvent> &channel_event_map) {
  Bucket &bucket = bucket_vec_[current_index_];
  Bucket::iterator iterator = bucket.begin();
  while (iterator != bucket.end()) {
    if (times_map_.find(*iterator) != times_map_.end()) {
      size_t &times = times_map_[*iterator];
      assert(times >= 1);
      if (times == 1) {
        channel_event_map[*iterator] |= EVENT_TIMEOVER;
        times_map_.erase(*iterator);
      }
      else {
        times -= 1;
      }
    }
    bucket.erase(iterator++);
  }
  current_index_++;
  current_index_ %= bucket_vec_.size();
}