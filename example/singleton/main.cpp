#include <cstdio>
#include "EventLoop/tool/Singleton.hpp"

class A : public Singleton<A> {
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
    A a;
    a.print();
    std::unique_ptr<A> &ptr1 = A::get_instance();
    ptr1->print();

    std::unique_ptr<A> &ptr2 = A::get_instance(1);
    ptr2->print();

    std::unique_ptr<A> &ptr3 = A::get_instance(2, 1);
    ptr3->print();

    std::unique_ptr<A> &ptr4 = A::get_instance(2, 2);
    ptr4->print();

    std::unique_ptr<A> &ptr5 = A::get_instance(2, 3);
    ptr5->print();

    std::unique_ptr<A> &ptr6 = A::get_instance(2, 4);
    ptr6->print();
}