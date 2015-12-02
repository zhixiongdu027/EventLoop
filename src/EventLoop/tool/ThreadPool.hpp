//
// Created by adugeek on 11/13/15.
//
#ifndef EVENTLOOP_TOOL_THREADPOOL_H
#define EVENTLOOP_TOOL_THREADPOOL_H

#include <vector>
#include <thread>

template<class T>
class ThreadPool {
public:
    template<typename... Args>
    void add_worker(Args &... args) {
        thread_vec_.push_back(T(args...));
    }

#if __cplusplus >= 201103L
    template<typename... Args>
    void add_worker(Args &&... args) noexcept {
        thread_vec_.emplace_back(std::move(T(args...)));
    }


#endif

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

#endif //EVNETLOOP_TOOL_THREADPOOL_H





