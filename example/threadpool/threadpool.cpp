//
// Created by adugeek on 12/16/15.
//


#include <EventLoop/tool/ThreadPool.h>
#include "EventLoop/tool/BlockingQueue.hpp"

BlockingQueue<int> queue;

static void my_work(int i) {
    while (true) {
        printf("id :%d ,val : %d\n", i, queue.pop());
    }
}


int main() {

    ThreadPool pool;
    auto function = my_work;
    for (int i = 0; i < 8; ++i) {
        pool.push_back(function, i);
    }

    for (int i = 0; i < 10000; ++i) {
        queue.push(i);
    }

    pool.join_all();
    return 0;
}