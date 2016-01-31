#include <iostream>
#include "EventLoop/tool/StreamBuffer.h"

int main() {
    StreamBuffer buffer;
    stream_buffer_append(&buffer, (uint16_t) 8, BlockData{"I", 1}, (uint32_t) 88, BlockData{"Love", 4}, (uint64_t) 888,
                         BlockData{"U", 1});

    size_t all = 0;
    uint16_t x;
    BlockData block1;
    uint32_t y;
    BlockData block2;
    uint64_t z;
    BlockData block3;

    StreamBuffer new_buffer(std::move(buffer));

    StreamBuffer new_new_buffer;

    std::swap(new_new_buffer, new_buffer);

    stream_buffer_quick_peek(&new_new_buffer, &all, &x, &block1, &y, &block2, &z, &block3);
    std::cout << std::string(block1.data, block1.len) << " " << std::string(block2.data, block2.len) << " " <<
    std::string(block3.data, block3.len) << std::endl;
    std::cout << all << " " << x << " " << y << " " << z << " " << std::endl;
}