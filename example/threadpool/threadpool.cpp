//
// Created by adugeek on 12/16/15.
//

#include "EventLoop/tool/BlockingQueue.hpp"
#include "EventLoop/tool/ThreadPool.hpp"

class Worker {
public:
    Worker(int id, BlockingQueue<int> &queue_) : id_(id), queue_(queue_) { };

    void operator()() {
        while (true) {
            int value = queue_.pop();
            printf("id : %d ,value : %d\n", id_, value);
        }
    }

private:
    int id_;
    BlockingQueue<int> &queue_;
};

int main() {

    BlockingQueue<int> queue;
    ThreadPool<Worker> pool;

    for (int i = 0; i < 4; ++i) {
        pool.add_worker(i, queue);
    }

    for (int i = 0; i < 1000000; ++i) {
        queue.push(i);
    }

    pool.join_all();
    printf("exit\n");
    return 0;
}