// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <algorithm>

/// +-------------------+------------------+------------------+
/// |    prependable    |     readable     |    writeable     |
/// |                   |                  |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
///memory       <=   peek_pos   <=     append_pos    <=   capacity

class StreamBuffer {
public:
    static size_t DefaultPrependable;
    static size_t DefaultCapacity;

public:
    StreamBuffer(size_t default_size = DefaultCapacity);

    StreamBuffer(const StreamBuffer &rhs);

    StreamBuffer &operator=(const StreamBuffer &rhs);;

    StreamBuffer(StreamBuffer &&rhs);

    StreamBuffer &operator=(StreamBuffer &&rhs);;

    ~StreamBuffer();

    inline void swap(StreamBuffer &rhs) {
        std::swap(memory_, rhs.memory_);
        std::swap(capacity_, rhs.capacity_);
        std::swap(peek_pos_, rhs.peek_pos_);
        std::swap(append_pos_, rhs.append_pos_);
    }

    inline const char *memory() const noexcept { return memory_; }

    inline size_t capacity() const noexcept { return capacity_; }

    inline void reserve(size_t len) {
        if (len > capacity_) {
            ensure_append_size(len - capacity_);
        }
    }

    inline size_t prependable() const noexcept { return peek_pos_; }

    inline size_t readable() const noexcept { return append_pos_ - peek_pos_; }

    inline size_t writeable() const noexcept { return capacity_ - append_pos_; }

    bool empty() const noexcept {
        return readable() == 0;
    }

    inline void discard_all() noexcept {
        peek_pos_ = DefaultPrependable;
        append_pos_ = DefaultPrependable;
    }

    inline void discard(size_t len) noexcept {
        if (len >= readable()) {
            discard_all();
        }
        else {
            peek_pos_ += len;
        }
    }

    inline void discard(size_t position, size_t len) noexcept {
        if (position == 0) {
            discard(len);
        }
        else if (position + len < readable()) {
            memmove(peek(position) , peek(position+len), readable() - position - len);
            append_pos_ -= len;
        }
        else if (position <= readable()) {
            append_pos_ = peek_pos_ + position;
            return;
        }
        else {
            assert(true); //if come here ,means  position > readable() ;
            return;
        }
    }

    inline void prepend(const void *data, size_t len) noexcept {
        assert(len <= prependable());
        peek_pos_ -= len;
        memcpy(peek(), data, len);
    }

    inline void prepend_uint8(uint8_t rhs) noexcept {
        assert(sizeof(uint8_t) < prependable());
        prepend(&rhs, sizeof(uint8_t));
    }

    inline void prepend_uint16(uint16_t rhs) noexcept {
        assert(sizeof(uint16_t) < prependable());
        uint16_t val = htobe16(rhs);
        prepend(&val, sizeof(uint16_t));
    }

    inline void prepend_uint32(uint32_t rhs) noexcept {
        assert(sizeof(uint32_t) < prependable());
        uint32_t val = htobe32(rhs);
        prepend(&val, sizeof(uint32_t));
    }

    inline void prepend_uint64(uint64_t rhs) noexcept {
        assert(sizeof(uint64_t) < prependable());
        uint64_t val = htobe64(rhs);
        prepend(&val, sizeof(uint64_t));
    }

    inline char *peek(size_t position = 0) noexcept {
        if (position < readable()) {
            return memory() + peek_pos_ + position;
        }
        return nullptr;
    }

    inline const char *peek(size_t position = 0) const noexcept {
        if (position < readable()) {
            return memory() + peek_pos_ + position;
        }
        return nullptr;
    }

    inline uint8_t peek_uint8(size_t position = 0) const noexcept {
        assert(position + sizeof(uint8_t) <= readable());
        uint8_t val = 0;
        ::memcpy(&val, peek(position), sizeof(uint8_t));
        return val;
    }

    inline uint16_t peek_uint16(size_t position = 0) const noexcept {
        assert(position + sizeof(uint16_t) <= readable());
        uint16_t val = 0;
        ::memcpy(&val, peek(position), sizeof(uint16_t));
        return be16toh(val);
    }

    inline uint32_t peek_uint32(size_t position = 0) const noexcept {
        assert(position + sizeof(uint32_t) <= readable());
        uint32_t val = 0;
        ::memcpy(&val, peek(position), sizeof(uint32_t));
        return be32toh(val);
    }

    inline uint64_t peek_uint64(size_t position = 0) const noexcept {
        assert(position + sizeof(uint64_t) <= readable());
        uint64_t val = 0;
        ::memcpy(&val, peek(position), sizeof(uint64_t));
        return be64toh(val);
    }

    inline void extract(size_t position, void *dst, size_t len) noexcept {
        assert(position + len <= readable());
        memcpy(dst, peek() + position, len);
        discard(position, len);
    }

    inline uint8_t extract_uint8(size_t position = 0) noexcept {
        uint8_t val;
        extract(position, &val, sizeof(uint8_t));
        return val;
    }

    inline uint16_t extract_uint16(size_t position = 0) noexcept {
        uint16_t val;
        extract(position, &val, sizeof(uint16_t));
        return be16toh(val);
    }

    inline uint32_t extract_uint32(size_t position = 0) noexcept {
        uint32_t val;
        extract(position, &val, sizeof(uint32_t));
        return be32toh(val);
    }

    inline uint64_t extract_uint64(size_t position = 0) noexcept {
        uint64_t val;
        extract(position, &val, sizeof(uint64_t));
        return be64toh(val);
    }

    inline void append(const void *data, size_t len) {
        ensure_append_size(len);
        memcpy(write_pos(), data, len);
        append_pos_ += len;
    }

    inline void append_uint8(uint8_t rhs) {
        append(&rhs, sizeof(uint8_t));
    }

    inline void append_uint16(uint16_t rhs) {
        uint16_t val = htobe16(rhs);
        append(&val, sizeof(uint16_t));
    }

    inline void append_uint32(uint32_t rhs) {
        uint32_t val = htobe32(rhs);
        append(&val, sizeof(uint32_t));
    }

    inline void append_uint64(uint64_t rhs) {
        uint64_t val = htobe64(rhs);
        append(&val, sizeof(uint64_t));
    }

    void insert(size_t position, const void *data, size_t len);

    inline void insert_uint8(size_t position, uint8_t rhs) noexcept {
        insert(position, &rhs, sizeof(uint8_t));
    }

    inline void insert_uint16(size_t position, uint16_t rhs) {
        uint16_t val = htobe16(rhs);
        insert(position, &val, sizeof(uint16_t));
    }

    inline void insert_uint32(size_t position, uint32_t rhs) {
        uint32_t val = htobe32(rhs);
        insert(position, &val, sizeof(uint32_t));
    }

    inline void insert_uint64(size_t position, uint64_t rhs) {
        uint64_t val = htobe64(rhs);
        insert(position, &val, sizeof(uint64_t));
    }

    void replace(size_t position, size_t replace_len, const void *data, size_t len);

    inline void replace_uint8(size_t position, size_t replace_len, uint8_t rhs) {
        replace(position, replace_len, &rhs, sizeof(uint8_t));
    }

    inline void replace_uint16(size_t position, size_t replace_len, uint16_t rhs) {
        uint16_t val = htobe16(rhs);
        replace(position, replace_len, &val, sizeof(uint16_t));
    }

    inline void replace_uint32(size_t position, size_t replace_len, uint32_t rhs) {
        uint32_t val = htobe32(rhs);
        replace(position, replace_len, &val, sizeof(uint32_t));
    }

    inline void replace_uint64(size_t position, size_t replace_len, uint64_t rhs) {
        uint64_t val = htobe64(rhs);
        replace(position, replace_len, &val, sizeof(uint64_t));
    }

    ssize_t read(int fd, size_t len) noexcept;

    ssize_t write(int fd ,size_t len) noexcept;

    ssize_t write(int fd, const void *data, size_t len) noexcept;

    ssize_t read_some(int fd) noexcept;

    ssize_t write_some(int fd) noexcept;

private:
    inline char *memory() noexcept { return memory_; }

    inline void *write_pos() noexcept { return memory() + append_pos_; }

    inline void ensure_append_size(size_t len) {
        assert(capacity_ >= append_pos_);
        assert(append_pos_ >= peek_pos_);
        if (writeable() < len) {
            ensure_append_size_with_memory_operator(len);
        }
    }

    void ensure_append_size_with_memory_operator(size_t len);

private:
    char *memory_;
    size_t capacity_;
    size_t peek_pos_;
    size_t append_pos_;
};

namespace std {
    template<>
    void swap(StreamBuffer &lhs, StreamBuffer &rhs);
};

#endif  // STREAMBUFFER_H