// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TOOL_SINGLETON_H
#define EVENTLOOP_TOOL_SINGLETON_H

#include <memory>
#include "Copyable.h"

template<typename T>
class Singleton : public NonCopyable {
protected:
    constexpr Singleton() = default;

public:
    template<typename... Args>
    static T &get_instance(Args &&... args) // Singleton
    {
        static T singleton(std::forward<Args>(args)...);
        return singleton;
    }
};

#endif //EVENTLOOP_TOOL_SINGLETON_H
