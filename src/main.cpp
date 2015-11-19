////简单来讲就是这样了。
////
////
////先定义 Pool
//#include <vector>
//#include <thread>
//#include <unistd.h>
//#include <iostream>
//#include <unordered_map>
//#include <unordered_set>
//
//
//
//
//template <class T>
//class Worker
//{
//public:
//    template <class ...Args>
//    void operator()(Args& ...args)
//    {
//        T(args...)->operator()();
//        worker_set_.insert(id_);
//    };
//    const int id_;
//    std::unordered_set<int> & worker_set_;
//    ~Worker(){};
//};
//
//template<class T>
//class WorkerPool {
//public:
//    template<typename... Args>
//    void add_worker(Args &... args)
//    {
//        for(auto& item :has_exit_)
//        {
//            thread_map_.erase(item);
//        }
//    }
//private:
//    std::unordered_map<std::thread::id, std::thread > thread_map_;
//    std::unordered_set<std::thread::id> has_exit_;
//};
//
////在定义一个 重载了operator()的类
//class A {
//public:
//    A(int i):id_(i){};
//    void operator()() {
//        while (true) {
//            sleep(1);
//            printf("i am running :%d\n",id_);
//        }
//    }
//    int id_;
//};
////        使用
//int main() {
//
//    WorkerPool<A> pool;
//    for (int i = 0; i < 3; ++i) {
//        pool.add_worker(i);
//    }
//    pool.detach();
//    sleep(5);
//}


