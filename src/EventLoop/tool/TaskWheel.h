// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TOOL_TASKWHEEL_H
#define EVENTLOOP_TOOL_TASKWHEEL_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

class TaskWheel {
public:
    explicit TaskWheel(size_t bucket_size) : bucket_vec_(bucket_size + 1), current_index_(0) {
    }

    inline void regist(size_t ticks, std::function<void()> cb) {
        size_t ticks_ = ticks > bucket_vec_.size() - 1 ? bucket_vec_.size() - 1 : ticks;
        auto &bucket = bucket_vec_[(current_index_ + ticks_) % bucket_vec_.size()];
        bucket.push_back(cb);
    }

    inline void tick() {
        auto &bucket = bucket_vec_[current_index_];
        for (auto item: bucket) {
            item();
        }
        bucket.clear();
        current_index_++;
        current_index_ %= bucket_vec_.size();
    }

private:
    TaskWheel(const TaskWheel &) = delete;

    TaskWheel &operator=(const TaskWheel &) = delete;

    std::vector<std::vector<std::function<void()> > > bucket_vec_;
    size_t current_index_;
};

#endif // EVENTLOOP_TOOL_TASKWHEEL_H