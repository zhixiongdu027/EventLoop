#include "EventLoop/tool/ConcurrentQueue.hpp"
#include "EventLoop/tool/WorkerPool.hpp"
#include <jsoncpp/json/json.h>
#include <unistd.h>


class Worker {
public:
    Worker(int id, ConcurrentQueue<std::shared_ptr<Json::Value> > &queue) : queue_(queue), id_(id) {
        static int i = 0;
        printf("num :%d\n", i++);
    };

    void operator()() {
        while (true) {
            std::shared_ptr<Json::Value> ptr = queue_.pop();
            if (ptr != nullptr) {
                printf("id :%d ,value : %d\n", id_, ptr->get("value", 0).asInt());
            }
        }
    }

    ConcurrentQueue<std::shared_ptr<Json::Value> > &queue_;
    int id_;
};

int main() {

    ConcurrentQueue<std::shared_ptr<Json::Value> > queue;
    WorkerPool<Worker> pool;
    for (int i = 0; i < 8; ++i) {
        pool.add_worker(i, queue);
    }
    pool.detach_all();

//    for(int i=0;i<10000000;++i){
//        std::shared_ptr<Json::Value> ptr(new Json::Value);
//        ptr->operator[]("value")=i;
//        queue.push(ptr);
//    }

    sleep(4);
}
