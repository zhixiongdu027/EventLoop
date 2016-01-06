// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_CHANNEL_H
#define EVENTLOOP_CHANNEL_H

#include "Forward.h"
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <unordered_map>
#include "tool/TaskWheel.h"
#include "tool/StreamBuffer.h"

class Channel {
    friend class EventLoop;

public:
    Context context;
    ContextDeleter context_deleter;

    inline int fd() const noexcept {
        return fd_;
    }

    inline ChannelId id() const noexcept {
        return id_;
    }

    inline void set_event_cb(ChannelCallback cb) noexcept {
        event_cb_ = cb;
    }

    inline StreamBuffer *get_read_buffer() noexcept {
        return &read_buffer_;
    }

    inline StreamBuffer *get_write_buffer() noexcept {
        return &write_buffer_;
    }

    inline void shutdown() noexcept {
        channel_event_map_[id_] |= TODO_SHUTDOWN;
    }

    int read() noexcept;

    inline void send() noexcept {
        is_socket_ ? send_to_socket() : send_to_normal();
    };

    inline void send(const char *data, size_t len) noexcept {
        is_socket_ ? send_to_socket(data, len) : send_to_normal(data, len);
    }

    ~Channel() noexcept {
        if (context_deleter != nullptr) {
            context_deleter(context.ptr);
        }
        close(fd_);
    };

private:
    static inline ChannelId make_channel_id() noexcept {
        static ChannelId id = 1;
        return __sync_fetch_and_add(&id, 1);
    }

    Channel(int fd, bool is_socket, bool is_nonblock, std::unordered_map<ChannelId, ChannelEvent> &event_map)
            : is_connected_(true), is_nonblock_(is_nonblock), is_socket_(is_socket), fd_(fd), id_(make_channel_id()),
              channel_event_map_(event_map) {
        memset(&context, 0x00, sizeof(context));
    }

    Channel(const Channel &rhs) = delete;

    Channel(Channel &&rhs) = delete;

    Channel &operator=(const Channel &rhs) = delete;

    void send_to_normal();

    void send_to_normal(const void *data, size_t len) noexcept;

    void send_to_socket();

    void send_to_socket(const void *data, size_t len) noexcept;

private:
    bool is_connected_;
    bool is_nonblock_;
    const bool is_socket_;
    int fd_;
    const ChannelId id_;
    std::unordered_map<ChannelId, ChannelEvent> &channel_event_map_;
    ChannelCallback event_cb_;
    StreamBuffer read_buffer_;
    StreamBuffer write_buffer_;
};

#endif // EVENTLOOP_CHANNEL_H
