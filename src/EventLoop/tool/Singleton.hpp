// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TOOL_SINGLETON_H
#define EVENTLOOP_TOOL_SINGLETON_H

#include <memory>
#include "Copyable.h"


template<typename T>
class Singleton : public NonCopyable {
public:
    template<typename... Args>
    static std::unique_ptr<T> &get_instance(Args &&... args) {
        static std::unique_ptr<T> singleton_ptr(new T(std::forward<Args>(args)...));
        return singleton_ptr;
    }
};

#endif //EVENTLOOP_TOOL_SINGLETON_H
