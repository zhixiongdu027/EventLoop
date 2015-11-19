//
// Created by adugeek on 11/13/15.
//
#ifndef EVENTLOOP_TOOL_THREADPOOL_H
#define EVENTLOOP_TOOL_THREADPOOL_H

#include <vector>
#include <thread>

template<class T>
class WorkerPool {
public:
    template<typename... Args>
    void add_worker(Args &... args) {
        thread_vec_.emplace_back(T(args...));
    }

private:
    std::vector<std::thread> thread_vec_;
};

#endif //EVNETLOOP_TOOL_THREAD_H





