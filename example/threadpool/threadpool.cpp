//
// Created by adugeek on 12/16/15.
//


#include <EventLoop/tool/ThreadPool.h>
#include "EventLoop/tool/BlockingQueue.hpp"

int main() {

    BlockingQueue<int> queue;

    auto fun1 = [&queue](int i) {
        while (true) {
            int val;
            if (queue.pop_wait_for(std::chrono::seconds(3), &val)) {
                printf("i am :fun1, id :%d ,val : %d\n", i, val);
                continue;
            };
            printf("%d will exit \n", i);
            break;
        };
    };

    auto fun2 = [&queue](int i) {
        while (true) {
            int val;
            if (queue.pop_wait_for(std::chrono::seconds(3), &val)) {
                printf("i am :fun2, id :%d ,val : %d\n", i, val);
                continue;
            };
            printf("%d will exit \n", i);
            break;
        };
    };

    ThreadPool pool;
    for (int i = 0; i < 10; ++i) {
        (i % 2 == 0) ? pool.push_back(fun1, i) : pool.push_back(fun2, i);
    }

    for (int i = 0; i < 1000000; ++i) {
        queue.emplace(i);
    }

    pool.join_all();
    return 0;
}