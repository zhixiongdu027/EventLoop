//
// Created by adugeek on 12/21/15.
//


#include <string>
#include <iostream>
#include "EventLoop/tool/SimpleEncode.h"

int main() {
    std::string buffer = "123456789-=qwertyuiop[]asdfghjkl;zxcvbnm,./";

    std::string begin = buffer;
    std::cout << "begin  : " << begin << std::endl;
    SimpleEncode::encode(&buffer);
    std::cout << "after encode : " << buffer << std::endl;
    SimpleEncode::decode(&buffer);
    std::cout << "after decode : " << buffer << std::endl;

    std::cout << " equal : " << (buffer == buffer) << std::endl;
    return 0;
}