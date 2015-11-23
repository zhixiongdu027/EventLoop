//
// Created by adugeek on 11/13/15.
//

#ifndef EVENTLOOP_TOOL_CONCURRENTQUEUE_H
#define EVENTLOOP_TOOL_CONCURRENTQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ConcurrentQueue {
public:

    void push(T &item) {
        {
            std::unique_lock<std::mutex> unique_lock(mutex_);
            queue_.push(item);
        }
        cond_.notify_one();
    }

    void push(T &&item) {
        {
            std::unique_lock<std::mutex> unique_lock(mutex_);
            queue_.emplace(std::move(item));
        }
        cond_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> unique_lock(mutex_);
        while (queue_.empty()) {
            cond_.wait(unique_lock);
        }
        T item(std::move(queue_.front()));
        queue_.pop();
        return item;
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

#endif  //EVENTLOOP_TOOL_CONCURRENTQUEUE_H