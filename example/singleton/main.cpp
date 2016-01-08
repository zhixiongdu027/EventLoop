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
    A &a1 = Singleton<A>::get_instance();
    a1.print();

    A &a2 = Singleton<A>::get_instance(1);
    a2.print();

    A &a3 = Singleton<A>::get_instance(2, 1);
    a3.print();

    A &a4 = Singleton<A>::get_instance(2, 2);
    a4.print();

    A &a5 = Singleton<A>::get_instance(2, 3);
    a5.print();

    A &a6 = Singleton<A>::get_instance(2, 4);
    a6.print();
}