//
// Created by adugeek on 12/16/15.
//


#include <EventLoop/tool/ThreadPool.h>
#include "EventLoop/tool/BlockingQueue.hpp"

BlockingQueue<int> queue;

void c_fun1(int i) {
    while (true) {
        int val;
        if (queue.pop_wait_for(std::chrono::seconds(3), &val)) {
            printf("i am :fun1, id :%d ,val : %d\n", i, val);
            continue;
        };
        printf("%d will exit \n", i);
        break;
    };
}

void c_fun2(int i) {
    while (true) {
        int val;
        if (queue.pop_wait_for(std::chrono::seconds(3), &val)) {
            printf("i am :fun2, id :%d ,val : %d\n", i, val);
            continue;
        };
        printf("%d will exit \n", i);
        break;
    };
}


int main() {

    ThreadPool pool;

    std::function<void(int)> fun1 = c_fun1;
    std::function<void(int)> fun2 = c_fun2;

    for (int i = 0; i < 10; ++i) {
        (i % 2 == 0) ? pool.emplace_back(c_fun1, i) : pool.emplace_back(c_fun2, i);
    }

    for (int i = 0; i < 1000000; ++i) {
        queue.push(i);
    }

    pool.join_all();
    return 0;
}