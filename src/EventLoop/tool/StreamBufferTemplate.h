//
// Created by adugeek on 1/28/16.
//

#ifndef EVENTLOOP_TOOL_STREAMBUFFERTEMPLATE_H
#define EVENTLOOP_TOOL_STREAMBUFFERTEMPLATE_H

#include "StreamBuffer.h"
#include "ExecuteState.h"

struct BlockData {
    BlockData() = default;

    // BlockData(char *data, size_t len) : data_(data), len_(len) { };
    BlockData(const char *data, size_t len) : data_(data), len_(len) { };
    const char *data_;
    size_t len_;
};

template<typename T>
void template_append(StreamBuffer *buffer, const T &t);

template<>
void template_append<uint8_t>(StreamBuffer *buffer, const uint8_t &t) {
    buffer->append_uint8(t);
}

template<>
void template_append<uint16_t>(StreamBuffer *buffer, const uint16_t &t) {
    buffer->append_uint16(t);
}

template<>
void template_append<uint32_t>(StreamBuffer *buffer, const uint32_t &t) {
    buffer->append_uint32(t);
}

template<>
void template_append<uint64_t>(StreamBuffer *buffer, const uint64_t &t) {
    buffer->append_uint64(t);
}

template<>
void template_append<BlockData>(StreamBuffer *buffer, const BlockData &t) {
    buffer->append_uint32((uint32_t) (sizeof(uint32_t) * 2 + t.len_));
    buffer->append_uint32((uint32_t) (sizeof(uint32_t) * 0 + t.len_));
    buffer->append(t.data_, t.len_);
}

template<typename T, typename ... Args>
void template_append(StreamBuffer *buffer, const T &t, Args ... args) {
    template_append(buffer, t);
    template_append(buffer, args...);
}

template<typename T>
ExecuteState template_peek(StreamBuffer *buffer, size_t *length, T *t);

template<>
ExecuteState template_peek(StreamBuffer *buffer, size_t *length, uint8_t *t) {
    if (buffer->peek_able() > *length + sizeof(uint8_t)) {
        *t = buffer->peek_uint8(*length);
        *length += sizeof(uint8_t);
        return ExecuteDone;
    }
    return ExecuteProcessing;
}

template<>
ExecuteState template_peek(StreamBuffer *buffer, size_t *length, uint16_t *t) {
    if (buffer->peek_able() > *length + sizeof(uint16_t)) {
        *t = buffer->peek_uint16(*length);
        *length += sizeof(uint16_t);
        return ExecuteDone;
    }
    return ExecuteProcessing;
}

template<>
ExecuteState template_peek(StreamBuffer *buffer, size_t *length, uint32_t *t) {
    if (buffer->peek_able() > *length + sizeof(uint32_t)) {
        *t = buffer->peek_uint32(*length);
        *length += sizeof(uint32_t);
        return ExecuteDone;
    }
    return ExecuteProcessing;
}

template<>
ExecuteState template_peek(StreamBuffer *buffer, size_t *length, uint64_t *t) {
    if (buffer->peek_able() > *length + sizeof(uint64_t)) {
        *t = buffer->peek_uint64(*length);
        *length += sizeof(uint64_t);
        return ExecuteDone;
    }
    return ExecuteProcessing;

}

template<>
ExecuteState template_peek(StreamBuffer *buffer, size_t *length, BlockData *t) {
    if (buffer->peek_able() < *length + sizeof(uint32_t) * 2) {
        return ExecuteProcessing;
    }
    uint32_t total_len = buffer->peek_uint32(*length + sizeof(uint32_t) * 0);
    uint32_t block_len = buffer->peek_uint32(*length + sizeof(uint32_t) * 1);
    if (total_len != block_len + sizeof(uint32_t) * 2) {
        return ExecuteError;
    }
    if (buffer->peek_able() < *length + total_len) {
        return ExecuteProcessing;
    }
    t->data_ = buffer->peek(*length + sizeof(uint32_t) * 2);
    t->len_ = block_len;
    *length += sizeof(uint32_t) * 2 + block_len;
    return ExecuteDone;
}

template<typename T, typename ... Args>
ExecuteState template_peek(StreamBuffer *buffer, size_t *length, T *t, Args ... args) {
    ExecuteState state;
    state = template_peek(buffer, length, t);
    if (state != ExecuteDone) {
        return state;
    }
    return template_peek(buffer, length, args...);
}

#endif //EVENTLOOP_TOOL_STREAMBUFFERTEMPLATE_H
