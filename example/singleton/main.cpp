#include <cstdio>
#include "EventLoop/tool/Singleton.hpp"

class A {
public:
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
    std::unique_ptr<A> &ptr = Singleton<A>::get_instance(1, 2);
    ptr->print();
}