// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include "StreamBuffer.h"

size_t StreamBuffer::DefaultPrependable = 12;
size_t StreamBuffer::DefaultCapacity = 1012;

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
    memcpy(peek(), rhs.peek(), rhs.peek_able());
    append_pos_ = peek_pos_ + rhs.peek_able();
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

StreamBuffer::StreamBuffer(StreamBuffer &&rhs) noexcept
        : memory_(rhs.memory_), capacity_(rhs.capacity_), peek_pos_(rhs.peek_pos_), append_pos_(rhs.append_pos_) {
    rhs.memory_ = nullptr;
    rhs.capacity_ = 0;
    rhs.peek_pos_ = 0;
    rhs.append_pos_ = 0;
}

StreamBuffer &StreamBuffer::operator=(StreamBuffer &&rhs) noexcept {
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
    if (moveable + append_able() >= len) {
        ::memmove(memory_ + DefaultPrependable, peek(), peek_able());
        peek_pos_ -= moveable;
        append_pos_ -= moveable;
        return;
    }

    size_t new_capacity = (len + capacity_) * 2;
    char *new_memory_ = new char[new_capacity];
    ::memcpy(new_memory_ + DefaultPrependable, peek(), peek_able());
    delete[] memory_;

    memory_ = new_memory_;
    capacity_ = new_capacity;

    ssize_t diff_len = peek_pos_ - DefaultPrependable; //may be less than zero

    peek_pos_ -= diff_len;
    append_pos_ -= diff_len;
    return;
}

void StreamBuffer::insert(size_t position, const void *data, size_t len) {
    assert(position <= peek_able());
    if (position == peek_able()) {
        append(data, len);
    }
    else if (len <= append_able()) {
        const char *move_src = peek(position);
        size_t move_len = peek_able() - position;
        char *move_dst = peek(position) + len;
        ::memmove(move_dst, move_src, move_len);
        ::memcpy(peek(position), data, len);
        append_pos_ += len;
    }
    else if (len <= prepend_able()) {
        const char *move_src = peek();
        size_t move_len = position;
        char *move_dst = peek() - len;
        ::memmove(move_dst, move_src, move_len);
        peek_pos_ -= len;
        ::memcpy(peek(position), data, len);
    }
    else if (len < prepend_able() + append_able()) {
        const char *right_move_src = peek(position);
        size_t right_move_len = peek_able() - position;
        char *right_move_dst = peek(position) + append_able();

        ::memmove(right_move_dst, right_move_src, right_move_len);

        const char *left_move_src = peek();
        size_t left_move_len = position;
        char *left_move_dst = peek() - (len - append_able());

        ::memmove(left_move_dst, left_move_src, left_move_len);

        peek_pos_ -= (len - append_able());
        ::memcpy(peek(position), data, len);
        append_pos_ = capacity();
    }
    else {
        size_t new_capacity = (len + capacity_) * 2;
        char *new_memory_ = new char[new_capacity];
        ::memcpy(new_memory_ + DefaultPrependable, peek(), position);
        ::memcpy(new_memory_ + DefaultPrependable + position, data, len);
        ::memcpy(new_memory_ + DefaultPrependable + position + len, peek(position), peek_able() - position);
        delete[] memory_;
        memory_ = new_memory_;
        capacity_ = new_capacity;
        append_pos_ = DefaultPrependable + peek_able() + len;
        peek_pos_ = DefaultPrependable;
    }
}

void StreamBuffer::replace(size_t position, size_t replace_len, const void *data, size_t len) {
    assert(position + replace_len <= peek_able());
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

ssize_t StreamBuffer::read(int fd, size_t len) noexcept {
    ensure_append_size(len);
    ssize_t read_res = ::read(fd, append_pos(), len);
    if (read_res > 0) {
        append_pos_ += read_res;
    }
    return read_res;
}

ssize_t StreamBuffer::write(int fd, size_t len) noexcept {
    assert(peek_able() >= len);
    ssize_t write_res = ::write(fd, peek(), len);
    if (write_res > 0) {
        discard(static_cast<size_t>(write_res));
    }
    return write_res;
}

ssize_t StreamBuffer::write(int fd, const void *data, size_t len) noexcept {
    assert(data != nullptr);
    ssize_t write_res;
    if (UNLIKELY(empty())) {
        write_res = ::write(fd, data, len);
    }
    else {
        iovec vec[2];
        vec[0].iov_base = peek();
        vec[0].iov_len = peek_able();
        vec[1].iov_base = (void *) data;
        vec[1].iov_len = len;
        write_res = ::writev(fd, &vec[0], 2);
    }
    if (UNLIKELY(write_res < 0)) {
        return write_res;
    }
    else if (static_cast<size_t>(write_res) >= peek_able()) {
        size_t use_data_len = static_cast<size_t>(write_res) - peek_able();
        discard_all();
        return use_data_len;
    }
    else {
        discard(static_cast<size_t>(write_res));
        return 0;
    }
}

ssize_t StreamBuffer::read_some(int fd) noexcept {
    constexpr size_t EXTRA_BUFF_SIZE = 65535;
    char extra_buff[EXTRA_BUFF_SIZE];
    struct iovec vec[2];
    vec[0].iov_base = append_pos();
    vec[0].iov_len = append_able();
    vec[1].iov_base = &extra_buff[0];
    vec[1].iov_len = EXTRA_BUFF_SIZE;

    ssize_t read_res = ::readv(fd, vec, 2);
    if (UNLIKELY(read_res < 0)) {
        return read_res;
    }
    else if (static_cast<size_t >(read_res) > append_able()) {
        size_t use_extra_len = static_cast<size_t >(read_res) - append_able();
        append_pos_ = capacity_;
        append(extra_buff, use_extra_len);
    }
    else {
        append_pos_ += read_res;
    }
    return read_res;
}

int StreamBuffer::read_n(int fd, size_t len, size_t *actual_read) {
    ensure_append_size(len);
    size_t had_read = 0;
    while (had_read < len) {
        ssize_t read_res = ::read(fd, append_pos(), len - had_read);
        if (UNLIKELY(read_res < 0)) {
            if (errno != EINTR && errno != EAGAIN) {
                if (actual_read != nullptr) {
                    *actual_read = had_read;
                }
                return -1;
            }
            continue;
        }
        else {
            had_read += read_res;
            append_pos_ += read_res;
        }
    }
    return 0;
}

int StreamBuffer::write_n(int fd, size_t len, size_t *actual_write) {
    assert(peek_able() >= len);
    size_t had_write = 0;
    while (had_write < len) {
        ssize_t write_res = ::write(fd, peek(), len - had_write);
        if (UNLIKELY(write_res < 0)) {
            if (errno != EINTR && errno != EAGAIN) {
                if (actual_write != nullptr) {
                    *actual_write = had_write;
                }
                return -1;
            }
            continue;
        }
        else {
            had_write += write_res;
            peek_pos_ += write_res;
        }
    }
    return 0;
}

template<>
void std::swap(StreamBuffer &lhs, StreamBuffer &rhs) noexcept {
    lhs.swap(rhs);
}