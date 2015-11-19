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
    T pop() {
        std::unique_lock<std::mutex> unique_lock(mutex_);
        while (queue_.empty()) {
            cond_.wait(unique_lock);
        }
        auto item = queue_.front();
        queue_.pop();
        return item;
    }

    void pop(T &item) {
        std::unique_lock<std::mutex> unique_lock(mutex_);
        while (queue_.empty()) {
            cond_.wait(unique_lock);
        }
        item = queue_.front();
        queue_.pop();
    }

    void push(T &item) {
        std::unique_lock<std::mutex> unique_lock(mutex_);
        queue_.push(item);
        unique_lock.unlock();
        cond_.notify_one();
    }

    void push(T &&item) {
        std::unique_lock<std::mutex> unique_lock(mutex_);
        queue_.push(std::move(item));
        unique_lock.unlock();
        cond_.notify_one();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif  //EVENTLOOP_TOOL_CONCURRENTQUEUE_H