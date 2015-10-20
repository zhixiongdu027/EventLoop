// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#include <sys/uio.h>
#include <unistd.h>
#include "StreamBuffer.h"

size_t StreamBuffer::DefaultPrependable = 12;
size_t StreamBuffer::DefaultCapacity = 1024;

StreamBuffer::StreamBuffer(size_t default_size) {
    memory_ = new char[default_size + DefaultPrependable];
    peek_pos_ = DefaultPrependable;
    append_pos_ = DefaultPrependable;
    capacity_ = default_size + DefaultPrependable;
}

StreamBuffer::StreamBuffer(const StreamBuffer &rhs) {
    capacity_ = rhs.capacity_;
    memory_ = new char[capacity_];
    if (rhs.peek_pos_ < DefaultPrependable) {
        peek_pos_ = rhs.peek_pos_;
    }
    else {
        peek_pos_ = DefaultPrependable;
    }
    memcpy(peek(), rhs.peek(), rhs.readable());
    append_pos_ = peek_pos_ + rhs.readable();
}

StreamBuffer &StreamBuffer::operator=(const StreamBuffer &rhs) {
    if (this != &rhs) {
        delete[] memory_;
        capacity_ = rhs.capacity_;
        peek_pos_ = rhs.peek_pos_;
        append_pos_ = rhs.append_pos_;
        memory_ = new char[capacity_];
        memcpy(memory_, rhs.memory_, rhs.capacity_);
    }
    return *this;
}

StreamBuffer::StreamBuffer(StreamBuffer &&rhs)
        : memory_(rhs.memory_), capacity_(rhs.capacity_), peek_pos_(rhs.peek_pos_), append_pos_(rhs.append_pos_) {
    rhs.memory_ = nullptr;
    rhs.capacity_ = 0;
    rhs.peek_pos_ = 0;
    rhs.append_pos_ = 0;
}

StreamBuffer &StreamBuffer::operator=(StreamBuffer &&rhs) {
    if (this != &rhs) {
        delete[]memory_;
        memory_ = rhs.memory_;
        capacity_ = rhs.capacity_;
        peek_pos_ = rhs.peek_pos_;
        append_pos_ = rhs.append_pos_;

        rhs.memory_ = nullptr;
        rhs.capacity_ = 0;
        rhs.peek_pos_ = 0;
        rhs.append_pos_ = 0;
    }
    return *this;
}

StreamBuffer::~StreamBuffer() {
    delete[] memory_;
    memory_ = nullptr;
}

void StreamBuffer::ensure_append_size_with_memory_operator(size_t len) {
    size_t moveable = (peek_pos_ > DefaultPrependable ? peek_pos_ - DefaultPrependable : 0);
    if (moveable + writeable() >= len) {
        ::memmove(memory_ + DefaultPrependable, peek(), readable());
        peek_pos_ -= moveable;
        append_pos_ -= moveable;
        return;
    }

    size_t new_capacity = (len + capacity_) * 2;
    char *new_memory_ = new char[new_capacity];
    ::memcpy(new_memory_ + DefaultPrependable, peek(), readable());
    delete[] memory_;

    memory_ = new_memory_;
    capacity_ = new_capacity;

    ssize_t diff_len = peek_pos_ - DefaultPrependable; //may be less than zero

    peek_pos_ -= diff_len;
    append_pos_ -= diff_len;
    return;
}

void StreamBuffer::insert(size_t position, const void *data, size_t len) {
    assert(position <= readable());
    if (position == readable()) {
        append(data, len);
    }
    else if (len <= writeable()) {
        const char *move_src = peek(position);
        size_t move_len = readable() - position;
        char *move_dst = peek(position) + len;
        ::memmove(move_dst, move_src, move_len);
        ::memcpy(peek(position), data, len);
        append_pos_ += len;
    }
    else if (len <= prependable()) {
        const char *move_src = peek();
        size_t move_len = position;
        char *move_dst = peek() - len;
        ::memmove(move_dst, move_src, move_len);
        peek_pos_ -= len;
        ::memcpy(peek(position), data, len);
    }
    else if (len < prependable() + writeable()) {
        const char *right_move_src = peek(position);
        size_t right_move_len = readable() - position;
        char *right_move_dst = peek(position) + writeable();

        ::memmove(right_move_dst, right_move_src, right_move_len);

        const char *left_move_src = peek();
        size_t left_move_len = position;
        char *left_move_dst = peek() - (len - writeable());

        ::memmove(left_move_dst, left_move_src, left_move_len);

        peek_pos_ -= (len - writeable());
        ::memcpy(peek(position), data, len);
        append_pos_ = capacity();
    }
    else {
        size_t new_capacity = (len + capacity_) * 2;
        char *new_memory_ = new char[new_capacity];
        ::memcpy(new_memory_ + DefaultPrependable, peek(), position);
        ::memcpy(new_memory_ + DefaultPrependable + position, data, len);
        ::memcpy(new_memory_ + DefaultPrependable + position + len, peek(position), readable() - position);
        delete[] memory_;
        memory_ = new_memory_;
        capacity_ = new_capacity;
        append_pos_ = DefaultPrependable + readable() + len;
        peek_pos_ = DefaultPrependable;
    }
}

void StreamBuffer::replace(size_t position, size_t replace_len, const void *data, size_t len) {
    assert(position + replace_len <= readable());
    if (replace_len == len) {
        ::memcpy(peek(position), data, len);
    }
    else if (replace_len > len) {
        ::memcpy(peek(position), data, len);
        discard(position + len, replace_len - len);
    }
    else if (replace_len < len) {
        ::memcpy(peek(position), data, replace_len);
        insert(position + replace_len, (const char *) data + replace_len, len - replace_len);
    }
}


ssize_t StreamBuffer::read(int fd, size_t len) noexcept{
    ensure_append_size(len);
    ssize_t last_read = ::read(fd, write_pos(), len);
    if(last_read>0) {
        append_pos_ += last_read;
    }
    return last_read;
}

ssize_t StreamBuffer::write(int fd, size_t len) noexcept{
    assert(len <= readable());
    ssize_t last_write = ::write(fd, peek(), len);
    if(last_write > 0) {
       discard(static_cast<size_t>(last_write));
    }
    return last_write;
}

ssize_t StreamBuffer::write(int fd, const void *data, size_t len) noexcept {
    assert(data != nullptr);
    iovec vec[2];
    vec[0].iov_base = peek();
    vec[0].iov_len = readable();
    vec[1].iov_base = (void *) data;
    vec[1].iov_len = len;

    ssize_t last_write = ::writev(fd, &vec[0], 2);
    if (last_write > readable())
    {
        size_t use_data_len = last_write-readable();
        discard_all();
        return use_data_len;
    }
    else if(last_write>0)
    {
        discard(last_write);
        return 0;
    }
    else
    {
        return last_write;
    }
}

ssize_t StreamBuffer::read_some(int fd) noexcept {
    constexpr size_t EXTRA_BUFF_SIZE = 65535;
    char extra_buff[EXTRA_BUFF_SIZE];
    struct iovec vec[2];
    vec[0].iov_base = write_pos();
    vec[0].iov_len = writeable();
    vec[1].iov_base = &extra_buff[0];
    vec[1].iov_len = EXTRA_BUFF_SIZE;

    ssize_t last_read = ::readv(fd, vec, 2);
    if (last_read > writeable()) {
        size_t use_extra_len = last_read - writeable();
        append_pos_ = capacity_;
        append(extra_buff, use_extra_len);
    }
    else if(last_read>0)
    {
        append_pos_ += last_read;
    }
    return last_read;
}

ssize_t StreamBuffer::write_some(int fd) noexcept {
    return write(fd, readable());
}

template<>
void std::swap(StreamBuffer &lhs, StreamBuffer &rhs) {
    lhs.swap(rhs);
}

