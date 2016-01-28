#include <iostream>
#include "EventLoop/tool/StreamBuffer.h"
#include "EventLoop/tool/StreamBufferTemplate.h"

int main() {

    StreamBuffer buffer;
    template_append(&buffer, (uint16_t) 8, BlockData{"I", 1}, (uint32_t) 88, BlockData{"Love", 4}, (uint64_t) 888,
                    BlockData{"U", 1});

    size_t all = 0;
    uint16_t x;
    BlockData block1;
    uint32_t y;
    BlockData block2;
    uint64_t z;
    BlockData block3;

    ExecuteState state = template_peek(&buffer, &all, &x, &block1, &y, &block2, &z, &block3);
    std::cout << state << std::endl;
    std::cout << std::string(block1.data_, block1.len_) << " " << std::string(block2.data_, block2.len_) << " " <<
    std::string(block3.data_, block3.len_) << std::endl;
    std::cout << all << " " << x << " " << y << " " << z << " " << std::endl;
}