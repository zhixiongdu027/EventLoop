//
// Created by adugeek on 12/21/15.
//


#include <string>
#include <iostream>
#include "EventLoop/tool/SimpleEncode.h"

int main() {
    std::string buffer;
    buffer.append("123456789", 9);

    std::cout << "begin  : " << buffer << std::endl;
    SimpleEncode::encode(&buffer);
    std::cout << "after encode : " << buffer << std::endl;
    SimpleEncode::decode(&buffer);
    std::cout << "after decode : " << buffer << std::endl;

    return 0;
}