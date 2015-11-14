//
// Created by adugeek on 11/13/15.
//

#include <vector>
#include <thread>

template<class T>
class WorkerPool {
public:
    template<typename... Args>
    void add_worker(Args &... __args) {
        t_vec_.emplace_back(__args...);
        thread_vec_.emplace_back(*t_vec_.rbegin());
    }

    template<typename... Args>
    void add_worker(Args &&... __args) {
        t_vec_.emplace_back(__args...);
        thread_vec_.emplace_back(*t_vec_.rbegin());
    }

    inline void join() {
        for (auto &item :thread_vec_) {
            item.join();
        }
    }

    inline void detach() {
        for (auto &item :thread_vec_) {
            item.detach();
        }
    }

private:
    std::vector<T> t_vec_;
    std::vector<std::thread> thread_vec_;
};








