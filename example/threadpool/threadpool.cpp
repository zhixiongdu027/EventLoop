//
// Created by adugeek on 12/16/15.
//


#include <EventLoop/tool/ThreadPool.h>
#include "EventLoop/tool/BlockingQueue.hpp"

BlockingQueue<int> queue;

static void my_work(int i) {
    while (true) {
        int val;
        if (queue.pop_until(std::chrono::system_clock::now() + std::chrono::seconds(3), &val)) {
            printf("id :%d ,val : %d\n", i, val);
            continue;
        };
        printf("%d will exit \n", i);
        break;
    }
}

int main() {

    ThreadPool pool;
    auto function = my_work;
    for (int i = 0; i < 10; ++i) {
        pool.push_back(function, i);
    }

    for (int i = 0; i < 100000; ++i) {
        queue.push(i);
    }

    pool.join_all();
    return 0;
}