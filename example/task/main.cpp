//
// Created by adugeek on 12/4/15.
//

#include <iostream>
#include "EventLoop/EventLoop.h"

void c_fun(EventLoopPtr &, void *, bool *again) {
    *again = true;
    printf("I am c_fun\n");
}

std::function<void(EventLoopPtr &, void *, bool *)> function = [](EventLoopPtr &, void *, bool *again) {
    *again = true;
    printf("I am function\n");
};

class A {
public:
    constexpr A() = default;

    void member(EventLoopPtr &, void *, bool *again) {
        *again = true;
        printf("I am member\n");
    }

    void operator()(EventLoopPtr &, void *, bool *again) {
        *again = true;
        printf("I am operator\n");
    }
};

int main() {
    EventLoop loop;
    loop.add_task_on_loop(true, 2, nullptr, c_fun);
    loop.add_task_on_loop(true, 2, nullptr, function);
    loop.add_task_on_loop(false, 2, nullptr, A());
    A a;
    loop.add_task_on_loop(false, 2, nullptr,
                          std::bind(&A::member, &a, std::placeholders::_1, std::placeholders::_2,
                                    std::placeholders::_3));
    loop.add_task_on_loop(false, 2, nullptr, [](EventLoopPtr &, void *, bool *again) {
        *again = false;
        printf("I am lam\n");
    });

    loop.start();
}