//
// Created by adugeek on 12/4/15.
//

#include <iostream>
#include "EventLoop/EventLoop.h"
#include <functional>

void fun(EventLoopPtr &, void *, bool *again) {
    std::cout << "I am here , See you after 5 seconds  @`_`@  ！" << std::endl;
    *again = true;
};


int main() {
    EventLoop loop;
    std::function<void(EventLoopPtr &, void *, bool *)> function = fun;
    loop.add_task_on_loop(true, 5, nullptr, function);
    loop.start();
}