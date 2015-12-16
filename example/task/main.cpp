//
// Created by adugeek on 12/4/15.
//

#include <iostream>
#include "EventLoop/EventLoop.h"

int main() {
    EventLoop loop;
    loop.add_task_on_loop(true, 5, nullptr, [](EventLoopPtr &, void *, bool *again) {
        std::cout << "I am here , See you after 5 seconds  @`_`@  ！" << std::endl;
        *again = true;
    });
    loop.start();
}