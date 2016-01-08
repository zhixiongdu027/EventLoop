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
            printf("id :%d ,val : %d ,label : %s\n", i, val, label);
            continue;
        };
        printf("%d will exit \n", i);
        break;
    };
}

class A {
public:
    void operator()(int i, const char *label) {
        while (true) {
            int val;
            if (queue.pop_wait_for(std::chrono::seconds(3), &val)) {
                printf("id :%d ,val : %d ,label : %s\n", i, val, label);
                continue;
            };
            printf("%d will exit \n", i);
            break;
        };
    }

    void member(int i, const char *label) {
        while (true) {
            int val;
            if (queue.pop_wait_for(std::chrono::seconds(3), &val)) {
                printf("id :%d ,val : %d ,label : %s\n", i, val, label);
                continue;
            };
            printf("%d will exit \n", i);
            break;
        };
    }
};

int main() {

    ThreadPool pool;

    A a;
    for (int i = 0; i < 4; ++i) {
        switch (i) {
            case 0:
                pool.emplace_back(c_fun1, i, "c_fun");
                break;
            case 1:
                pool.emplace_back(A(), i, "operator");
                break;
            case 2:
                pool.emplace_back(std::bind(&A::member, &a, i, "member"));
                break;
            case 3:
                pool.emplace_back([](int i, const char *label) {
                    while (true) {
                        int val;
                        if (queue.pop_wait_for(std::chrono::seconds(3), &val)) {
                            printf("id :%d ,val : %d ,label : %s\n", i, val, label);
                            continue;
                        };
                        printf("%d will exit \n", i);
                        break;
                    };
                }, i, "lama");
                break;
            default:
                break;
        }
    }

    for (int i = 0; i < 1000000; ++i) {
        queue.push(i);
    }
    pool.join_all();
    return 0;
}