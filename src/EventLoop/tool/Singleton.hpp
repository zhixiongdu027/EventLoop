// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TOOL_SINGLETON_H
#define EVENTLOOP_TOOL_SINGLETON_H

#include <memory>

template<typename T> // Singleton policy class
class Singleton {
protected:
    Singleton() = default;

    virtual ~Singleton() = default;

    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton &) = delete;

public:
    template<typename... Args>
    static std::unique_ptr<T> &get_instance(Args... args) // Singleton
    {
        static std::unique_ptr<T> singleton_ptr(new T(std::forward<Args>(args)...));
        return singleton_ptr;
    }
};


#endif //EVENTLOOP_TOOL_SINGLETON_H
