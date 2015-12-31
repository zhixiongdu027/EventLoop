// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TOOL_THREADPOOL_H
#define EVENTLOOP_TOOL_THREADPOOL_H

#include <vector>
#include <thread>
#include "Copyable.h"

class ThreadPool : public NonCopyable {
public:
    ThreadPool() = default;

    void push_back(std::thread &&thread) noexcept {
        thread_vec_.push_back(std::forward<std::thread &&>(thread));
    }

    template<typename... Args>
    void push_back(const Args &... args) noexcept {
        thread_vec_.emplace_back(std::forward<const Args &>(args)...);
    }

    template<typename... Args>
    void emplace_back(Args &&... args) noexcept {
        thread_vec_.emplace_back(std::forward<Args &&>(args)...);
    }

    inline void detach_all() {
        for (auto &item : thread_vec_) {
            item.detach();
        }
    }

    inline void join_all() {
        for (auto &item :thread_vec_) {
            if (item.joinable()) {
                item.join();
            }
        }
    }

private:
    std::vector<std::thread> thread_vec_;
};

#endif //EVENTLOOP_THREADPOOL_H
