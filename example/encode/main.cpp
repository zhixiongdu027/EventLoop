//
// Created by adugeek on 12/21/15.
//

#include <fcntl.h>
#include "EventLoop/tool/StreamBuffer.h"
#include "EventLoop/tool/SimpleEncode.h"

int main() {
    int read_fd = open("test.txt", O_RDONLY);

    StreamBuffer buffer;
    while (buffer.read_some(read_fd) > 0) { };

    SimpleEncode::encode(buffer.peek(), buffer.peek_able());
    SimpleEncode::decode(buffer.peek(), buffer.peek_able());

    int out_fd = open("test_out.txt", O_CREAT | O_WRONLY, 0666);
    while (buffer.write_some(out_fd) > 0) { };

    return 0;
}