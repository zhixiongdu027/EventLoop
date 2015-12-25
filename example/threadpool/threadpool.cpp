//
// Created by adugeek on 12/16/15.
//


#include <EventLoop/tool/ThreadPool.h>
#include "EventLoop/tool/BlockingQueue.hpp"

int main() {

    BlockingQueue<int> queue;
    ThreadPool pool;

    for (int i = 0; i < 8; ++i) {
        pool.emplace_back([&queue, i] {
            while (true) {
                printf("id :%d ,val : %d\n", i, queue.pop());
            }
        });
    }

    for (int i = 0; i < 10000; ++i) {
        queue.push(i);
    }

    pool.join_all();
    return 0;
}