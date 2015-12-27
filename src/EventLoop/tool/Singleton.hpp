//
// Created by adugeek on 12/27/15.
//

#ifndef EVENTLOOP_TOOL_SINGLETON_H
#define EVENTLOOP_TOOL_SINGLETON_H

#include <memory>

template<typename T>
class Singleton {
public:
    template<typename... Args>
    static std::unique_ptr<T> &get_instance(Args &&... args) {
        static std::unique_ptr<T> singleton_ptr(new T(std::forward<Args>(args)...));
        return singleton_ptr;
    }
};

#endif //EVENTLOOP_TOOL_SINGLETON_H
