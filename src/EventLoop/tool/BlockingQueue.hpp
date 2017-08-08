// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TOOL_BLOCKINGQUEUE_H
#define EVENTLOOP_TOOL_BLOCKINGQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include "Copyable.h"

template<typename T>
class BlockingQueue : public NonCopyable {
public:
    BlockingQueue<T>() = default;

    void push(const T &item) {
        {
            std::unique_lock<std::mutex> unique_lock(mutex_);
            queue_.push(item);
        }
        cond_.notify_one();
    }

    void push(T &&item) {
        {
            std::unique_lock<std::mutex> unique_lock(mutex_);
            queue_.push(std::move(item));
        }
        cond_.notify_one();
    }

    template<typename... Args>
    void emplace(Args &&... args) {
        {
            std::unique_lock<std::mutex> unique_lock(mutex_);
            queue_.emplace(std::forward<Args>(args)...);
        }
        cond_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> unique_lock(mutex_);
        cond_.wait(unique_lock, [this] { return !queue_.empty(); });
        T item(std::move(queue_.front()));
        queue_.pop();
        return item;
    }

    template<typename _Rep, typename _Period>
    bool pop_wait_for(const std::chrono::duration<_Rep, _Period> &relative_time, T *value) {
        assert(value != nullptr);
        std::unique_lock<std::mutex> unique_lock(mutex_);
        if (cond_.wait_for(unique_lock, relative_time, [this] { return !queue_.empty(); })) {
            *value = std::move(queue_.front());
            queue_.pop();
            return true;
        }
        else {
            return false;
        }
    }

    template<typename _Clock, typename _Duration>
    bool pop_wait_until(const std::chrono::time_point<_Clock, _Duration> &absolute_time, T *value) {
        assert(value != nullptr);
        std::unique_lock<std::mutex> unique_lock(mutex_);
        if (cond_.wait_until(unique_lock, absolute_time, [this] { return !queue_.empty(); })) {
            *value = std::move(queue_.front());
            queue_.pop();
            return true;
        }
        else {
            return false;
        }
    }

    size_t size() const {
        std::unique_lock<std::mutex> unique_lock(mutex_);
        return queue_.size();
    }

private:

    mutable std::mutex mutex_;
    std::condition_variable cond_;
    std::queue<T> queue_;
};

#endif  //EVENTLOOP_TOOL_BLOCKINGQUEUE_H