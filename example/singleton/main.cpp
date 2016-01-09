#include <cstdio>
#include "EventLoop/tool/Singleton.hpp"

class A {
public:
    A() : a_(0), b_(0) { };

    A(int a) : a_(a), b_(0) { };

    A(int a, int b) : a_(a), b_(b) { };

    void print() {
        printf("a : %d ,b: %d\n", a_, b_);
    }

    ~A() {
        printf("erase A\n");
    }

private:
    int a_;
    int b_;
};

int main() {
    Singleton<A>::init(2, 4);
    Singleton<A>::init(3, 8);
    Singleton<A>::init(1, 2);
    auto &a1 = Singleton<A>::instance();
    a1->print();

}