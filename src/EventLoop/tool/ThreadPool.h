//
// Created by adugeek on 12/25/15.
//

#ifndef EVENTLOOP_TOOL_THREADPOOL_H
#define EVENTLOOP_TOOL_THREADPOOL_H

#include <vector>
#include <thread>

class ThreadPool {
public:
    ThreadPool() = default;

    ThreadPool(const ThreadPool &) = delete;

    void push_back(std::thread &&thread) noexcept {
        thread_vec_.push_back(std::move(thread));
    }

    template<typename... Args>
    void push_back(const Args &... args) noexcept {
        thread_vec_.push_back(std::thread(args...));
    }

    template<typename... Args>
    void emplace_back(Args &&... args) noexcept {
        thread_vec_.emplace_back(std::move(args...));
    }

    inline void detach_all() {
        for (auto &item : thread_vec_) {
            item.detach();
        }
    }

    inline void join_all() {
        for (auto &item :thread_vec_) {
            item.join();
        }
    }

private:

    std::vector<std::thread> thread_vec_;
};

#endif //EVENTLOOP_THREADPOOL_H
