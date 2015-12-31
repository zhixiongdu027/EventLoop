#include <cstdio>
#include "EventLoop/tool/Singleton.hpp"

class A {
public:
    A(int a, int b) : a_(a), b_(b) {};

    A() : a_(1), b_(1) { };

    A(int a) : a_(a), b_() { };

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
    std::unique_ptr<A> &ptr1 = Singleton<A>::get_instance();
    ptr1->print();

    std::unique_ptr<A> &ptr2 = Singleton<A>::get_instance(1);
    ptr2->print();

    std::unique_ptr<A> &ptr3 = Singleton<A>::get_instance(1, 2);
    ptr3->print();

}