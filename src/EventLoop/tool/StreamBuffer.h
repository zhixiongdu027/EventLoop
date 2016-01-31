// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TOOL_STREAMBUFFER_H
#define EVENTLOOP_TOOL_STREAMBUFFER_H


#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <algorithm>
#include "Likely.h"
#include "ExecuteState.h"
/// +-------------------+------------------+------------------+
/// |    prepend_able   |     peek_able    |    append_able   |
/// |                   |                  |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
///memory       <=   peek_pos   <=     append_pos    <=   capacity

struct BlockData {
    BlockData() = default;

    BlockData(const char *data, size_t len) : data(data), len(len) { };
    const char *data;
    size_t len;
};

class StreamBuffer {
public:
    static size_t DefaultPrependable;
    static size_t DefaultCapacity;

public:
    explicit StreamBuffer(size_t default_size = DefaultCapacity);

    StreamBuffer(const StreamBuffer &rhs);

    StreamBuffer &operator=(const StreamBuffer &rhs);

    StreamBuffer(StreamBuffer &&rhs) noexcept;

    StreamBuffer &operator=(StreamBuffer &&rhs) noexcept;

    ~StreamBuffer();

    inline void swap(StreamBuffer &rhs) noexcept {
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

    inline size_t prepend_able() const noexcept { return peek_pos_; }

    inline size_t peek_able() const noexcept { return append_pos_ - peek_pos_; }

    inline size_t append_able() const noexcept { return capacity_ - append_pos_; }

    bool empty() const noexcept {
        return peek_able() == 0;
    }

    inline void discard_all() noexcept {
        peek_pos_ = DefaultPrependable;
        append_pos_ = DefaultPrependable;
    }

    inline void discard(size_t len) noexcept {
        if (LIKELY(len < peek_able())) {
            peek_pos_ += len;
        }
        else {
            discard_all();
        }
    }

    inline void discard(size_t position, size_t len) noexcept {
        if (position == 0) {
            discard(len);
        }
        else if (LIKELY(position + len < peek_able())) {
            memmove(peek(position), peek(position + len), peek_able() - position - len);
            append_pos_ -= len;
        }
        else if (LIKELY(position < peek_able())) {
            append_pos_ = peek_pos_ + position;
            return;
        }
        else {
            assert(true); //if come here ,means  position >= peek_able() ;
            return;
        }
    }

    inline void prepend(const void *data, size_t len) noexcept {
        assert(len <= prepend_able());
        peek_pos_ -= len;
        memcpy(peek(), data, len);
    }

    inline void prepend_uint8(uint8_t rhs) noexcept {
        prepend(&rhs, sizeof(uint8_t));
    }

    inline void prepend_uint16(uint16_t rhs) noexcept {
        uint16_t val = htobe16(rhs);
        prepend(&val, sizeof(uint16_t));
    }

    inline void prepend_uint32(uint32_t rhs) noexcept {
        uint32_t val = htobe32(rhs);
        prepend(&val, sizeof(uint32_t));
    }

    inline void prepend_uint64(uint64_t rhs) noexcept {
        uint64_t val = htobe64(rhs);
        prepend(&val, sizeof(uint64_t));
    }

    inline char *peek(size_t position = 0) noexcept {
        if (position < peek_able()) {
            return memory() + peek_pos_ + position;
        }
        return nullptr;
    }

    inline char *peek_begin()noexcept {
        return memory() + peek_pos_;
    }

    inline char *peek_end() noexcept {
        return memory() + append_pos_;
    }

    inline const char *peek(size_t position = 0) const noexcept {
        if (position < peek_able()) {
            return memory() + peek_pos_ + position;
        }
        return nullptr;
    }

    inline uint8_t peek_uint8(size_t position = 0) const noexcept {
        assert(position + sizeof(uint8_t) <= peek_able());
        uint8_t val = 0;
        ::memcpy(&val, peek(position), sizeof(uint8_t));
        return val;
    }

    inline uint16_t peek_uint16(size_t position = 0) const noexcept {
        assert(position + sizeof(uint16_t) <= peek_able());
        uint16_t val = 0;
        ::memcpy(&val, peek(position), sizeof(uint16_t));
        return be16toh(val);
    }

    inline uint32_t peek_uint32(size_t position = 0) const noexcept {
        assert(position + sizeof(uint32_t) <= peek_able());
        uint32_t val = 0;
        ::memcpy(&val, peek(position), sizeof(uint32_t));
        return be32toh(val);
    }

    inline uint64_t peek_uint64(size_t position = 0) const noexcept {
        assert(position + sizeof(uint64_t) <= peek_able());
        uint64_t val = 0;
        ::memcpy(&val, peek(position), sizeof(uint64_t));
        return be64toh(val);
    }

    inline void extract(size_t position, void *dst, size_t len) noexcept {
        assert(position + len <= peek_able());
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
        memcpy(append_pos(), data, len);
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

    ssize_t write(int fd, size_t len) noexcept;

    ssize_t write(int fd, const void *data, size_t len) noexcept;

    ssize_t read_some(int fd) noexcept;

    inline ssize_t write_some(int fd) noexcept {
        return write(fd, peek_able());
    }

    int read_n(int fd, size_t len, size_t *actual_read);

    int write_n(int fd, size_t len, size_t *actual_write);

    inline int write_all(int fd, size_t *actual_write) {
        return write_n(fd, peek_able(), actual_write);
    }

private:
    inline char *memory() noexcept { return memory_; }

    inline void *append_pos() noexcept { return memory() + append_pos_; }

    inline void ensure_append_size(size_t len) {
        assert(capacity_ >= append_pos_);
        assert(append_pos_ >= peek_pos_);
        if (append_able() < len) {
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
    void swap(StreamBuffer &lhs, StreamBuffer &rhs) noexcept;
};

template<typename T>
void stream_buffer_append(StreamBuffer *buffer, const T &t);

template<>
inline void stream_buffer_append<uint8_t>(StreamBuffer *buffer, const uint8_t &t) {
    buffer->append_uint8(t);
}

template<>
inline void stream_buffer_append<uint16_t>(StreamBuffer *buffer, const uint16_t &t) {
    buffer->append_uint16(t);
}

template<>
inline void stream_buffer_append<uint32_t>(StreamBuffer *buffer, const uint32_t &t) {
    buffer->append_uint32(t);
}

template<>
inline void stream_buffer_append<uint64_t>(StreamBuffer *buffer, const uint64_t &t) {
    buffer->append_uint64(t);
}

template<>
inline void stream_buffer_append<BlockData>(StreamBuffer *buffer, const BlockData &t) {
    buffer->append_uint32((uint32_t) (sizeof(uint32_t) * 2 + t.len));
    buffer->append_uint32((uint32_t) (sizeof(uint32_t) * 0 + t.len));
    buffer->append(t.data, t.len);
}

template<typename T, typename ... Args>
void stream_buffer_append(StreamBuffer *buffer, const T &t, Args &&... args) {
    stream_buffer_append(buffer, t);
    stream_buffer_append(buffer, std::forward<Args>(args)...);
}

template<typename T>
void stream_buffer_quick_peek(StreamBuffer *buffer, size_t *length, T *t);

template<>
inline void stream_buffer_quick_peek<uint8_t>(StreamBuffer *buffer, size_t *length, uint8_t *t) {
    *t = buffer->peek_uint8(*length);
    *length += sizeof(uint8_t);
}

template<>
inline void stream_buffer_quick_peek<uint16_t>(StreamBuffer *buffer, size_t *length, uint16_t *t) {
    *t = buffer->peek_uint16(*length);
    *length += sizeof(uint16_t);
}

template<>
inline void stream_buffer_quick_peek<uint32_t>(StreamBuffer *buffer, size_t *length, uint32_t *t) {
    *t = buffer->peek_uint32(*length);
    *length += sizeof(uint32_t);
}

template<>
inline void stream_buffer_quick_peek<uint64_t>(StreamBuffer *buffer, size_t *length, uint64_t *t) {
    *t = buffer->peek_uint64(*length);
    *length += sizeof(uint64_t);
}

template<>
inline void stream_buffer_quick_peek<BlockData>(StreamBuffer *buffer, size_t *length, BlockData *t) {
    uint32_t block_len = buffer->peek_uint32(*length + sizeof(uint32_t) * 1);
    t->data = buffer->peek(*length + sizeof(uint32_t) * 2);
    t->len = block_len;
    *length += sizeof(uint32_t) * 2 + block_len;
}

template<typename T, typename ... Args>
void stream_buffer_quick_peek(StreamBuffer *buffer, size_t *length, T *t, Args &&... args) {
    stream_buffer_peek(buffer, length, t);
    stream_buffer_peek(buffer, length, std::forward<Args>(args)...);
}


template<typename T>
ExecuteState stream_buffer_peek(StreamBuffer *buffer, size_t *length, T *t);

template<typename T, typename ... Args>
ExecuteState stream_buffer_peek(StreamBuffer *buffer, size_t *length, T *t, Args &&... args) {
    ExecuteState state;
    state = stream_buffer_peek(buffer, length, t);
    if (state != ExecuteDone) {
        return state;
    }
    return stream_buffer_peek(buffer, length, std::forward<Args>(args)...);
}


#endif  // EVENTLOOP_TOOL_STREAMBUFFER_H