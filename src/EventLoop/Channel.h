// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_CHANNEL_H
#define EVENTLOOP_CHANNEL_H

#include "Forward.h"
#include <unordered_map>
#include <sys/socket.h>
#include <unistd.h>
#include "StreamBuffer.h"
#include "TimerWheel.h"

class Channel {
    friend class EventLoop;

public:
    void *context;
    std::function<void(void *)> delete_context;

    inline int fd() const noexcept {
        return fd_;
    }

    inline ChannelId id() const noexcept {
        return id_;
    }

    inline void set_event_cb(ChannelCallback &cb) noexcept {
        event_cb_ = cb;
    }

    inline void set_nonblock(bool is_nonblock) noexcept {
        is_nonblock_ = is_nonblock;
    }

    inline StreamBuffer *get_read_buffer() noexcept {
        return &readBuffer_;
    }

    inline StreamBuffer *get_write_buffer() noexcept {
        return &writeBuffer_;
    }

    inline void add_live_time(size_t seconds) noexcept {
        timer_wheel_.regist(id() ,seconds);
    }

    inline void shutdown() noexcept {
        channel_event_map_[id_] |= TODO_SHUTDOWN;
    }

    inline void erase() noexcept {
        channel_event_map_[id_] |= TODO_ERASE;
    }

    int read() noexcept;

    void send() noexcept;

    inline void send(const char *data, size_t len) noexcept {
        is_socket_ ? send_to_socket(data, len) : send_to_normal(data, len);
    }

    ~Channel() noexcept {
        if (context != nullptr && delete_context != nullptr) {
            delete_context(context);
        }
        close(fd_);
    };

private:
    static inline ChannelId make_channel_id() noexcept {
        static ChannelId id = 1;
        return __sync_fetch_and_add(&id, 1);
    }

    Channel(int fd,
            bool is_socket, std::unordered_map<ChannelId, ChannelEvent> &event_map ,TimerWheel& timer_wheel)
            : context(nullptr), id_(make_channel_id()), fd_(fd), is_socket_(is_socket),
              is_connected_(true), is_nonblock_(false),
              channel_event_map_(event_map) ,timer_wheel_(timer_wheel){
    }

    Channel(const Channel &rhs) = delete;

    Channel(Channel &&rhs) = delete;

    Channel &operator=(const Channel &rhs) = delete;

    void send_to_normal(const void *data, size_t len) noexcept;

    void send_to_socket(const void *data, size_t len) noexcept;

private:
    const ChannelId id_;
    const int fd_;
    const bool is_socket_;
    bool is_connected_;
    bool is_nonblock_;
    std::unordered_map<ChannelId, ChannelEvent> &channel_event_map_;
    TimerWheel &timer_wheel_;
    ChannelCallback event_cb_;
    StreamBuffer readBuffer_;
    StreamBuffer writeBuffer_;
};

#endif // EVENTLOOP_CHANNEL_H
