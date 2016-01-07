//
// Created by adugeek on 12/16/15.
//


#include <EventLoop/tool/ThreadPool.h>
#include "EventLoop/tool/BlockingQueue.hpp"

BlockingQueue<int> queue;

void c_fun1(int i, const char *label) {
    while (true) {
        int val;
        if (queue.pop_wait_for(std::chrono::seconds(3), &val)) {
            printf("i am :fun1, id :%d ,val : %d ,label : %s\n", i, val, label);
            continue;
        };
        printf("%d will exit \n", i);
        break;
    };
}

void c_fun2(int i, const char *label) {
    while (true) {
        int val;
        if (queue.pop_wait_for(std::chrono::seconds(3), &val)) {
            printf("i am :fun2, id :%d ,val : %d ,label : %s\n", i, val, label);
            continue;
        };
        printf("%d will exit \n", i);
        break;
    };
}

int main() {

    ThreadPool pool;

    std::function<void(int, const char *)> fun1 = c_fun1;
    std::function<void(int, const char *)> fun2 = c_fun2;

    for (int i = 0; i < 10; ++i) {
        if (i % 2 == 0) {
            std::thread temp(fun1, i, "number 1");
            pool.push_back(std::move(temp));
        }
        else {
            std::thread temp(fun2, i, "number 2");
            pool.push_back(std::move(temp));
        };
    }

    for (int i = 0; i < 1000000; ++i) {
        queue.push(i);
    }
    pool.join_all();
    return 0;
}