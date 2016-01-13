// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TOOL_SINGLETON_H
#define EVENTLOOP_TOOL_SINGLETON_H

#include <memory>
#include <mutex>
#include <assert.h>

template<typename T>
class Singleton {
    Singleton() = delete;

    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton &) = delete;

public:
    template<typename... Args>
    static void init(Args &&... args) {
        std::call_once(flag_, [](Args &&... args) {
            {
                std::unique_ptr<T> temp(new T(std::forward<Args>(args)...));
                singleton_.swap(temp);
            }
        }, std::forward<Args>(args)...);
    }

    static std::unique_ptr<T> &instance() {
        assert(singleton_ != nullptr);
        return singleton_;
    }

private:
    static std::unique_ptr<T> singleton_;
    static std::once_flag flag_;
};

template<typename T> std::unique_ptr<T> Singleton<T>::singleton_;
template<typename T> std::once_flag Singleton<T>::flag_;

#endif //EVENTLOOP_TOOL_SINGLETON_H
