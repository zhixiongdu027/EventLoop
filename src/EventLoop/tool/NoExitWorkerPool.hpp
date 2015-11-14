//
// Created by adugeek on 11/13/15.
//

#include <vector>
#include <thread>

template<class T>
class NoExitWorkerPool {
public:
    template<typename... Args>
    void add_worker(Args &... __args) {
        thread_vec_.emplace_back(T(__args...));
    }

    template<typename... Args>
    void add_worker(Args &&... __args) {
        thread_vec_.emplace_back(T(__args...));
    }

    inline void join_all() {
        for (auto &item :thread_vec_) {
            item.join();
        }
    }

    inline void detach_all() {
        for (auto &item :thread_vec_) {
            item.detach();
        }
    }

    inline size_t worker_size() {
        return thread_vec_.size();
    }

private:
    std::vector<std::thread> thread_vec_;
};








