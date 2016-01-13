// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_EVENTLOOP_H
#define EVENTLOOP_EVENTLOOP_H

#include <string.h>
#include <sys/epoll.h>
#include <unordered_map>
#include "Forward.h"
#include "Channel.h"
#include "tool/TaskWheel.h"

class EventLoop : public NonCopyable {
public:
    EventLoop() : init_status_(INIT_STATUS::INIT), epoll_(-1), timer_(-1), quit_(true), task_wheel_(300) {
        memset(&context, 0x00, sizeof(context));
    }

    ~EventLoop();

    ChannelPtr &add_channel(int fd, bool is_socket, bool is_nonblock, ssize_t lifetime, ChannelCallback io_event_cb);

    ChannelPtr &add_connecting_channel(int fd, ssize_t connect_time, ssize_t lifetime, ChannelCallback io_event_cb);

    inline void erase_channel(ChannelId id) noexcept {
        channel_event_map_[id] |= TODO_ERASE;
    }

    inline ChannelPtr &get_channel(const ChannelId id) noexcept {
        return (channel_map_.find(id) != channel_map_.end()) ? channel_map_[id] : null_channel_ptr;
    }

    inline void add_task_on_loop(bool imd_exec, size_t seconds, void *user_arg,
                                 const std::function<void(EventLoopPtr &, void *user_arg, bool *again)> &cb) {
        if (init_status_ == INIT_STATUS::INIT) {
            init();
        }
        bool again = true;
        if (imd_exec) {
            cb(this, user_arg, &again);
        }
        if (LIKELY(again)) {
            add_task_on_loop_recursive(seconds, user_arg, std::move(cb));
        }
    }


    inline void add_task_on_channel(bool imd_exec, ChannelId channel_id, size_t seconds, void *user_arg,
                                    const std::function<void(EventLoopPtr &, ChannelPtr &, void *user_arg,
                                                             bool *again)> &cb) {
        if (channel_map_.find(channel_id) != channel_map_.end()) {
            bool again = true;
            if (imd_exec) {
                cb(this, channel_map_[channel_id], user_arg, &again);
            }
            if (LIKELY(again)) {
                add_task_on_channel_recursive(channel_id, seconds, user_arg, std::move(cb));
            }
        }
    }


    inline void add_channel_lifetime(ChannelId channel_id, size_t seconds) {
        if (channel_map_.find(channel_id) != channel_map_.end()) {
            channel_lives_map_[channel_id]++;
            task_wheel_.regist(seconds,
                               [this, channel_id]() {
                                   size_t channel_lives = channel_lives_map_[channel_id];
                                   if (UNLIKELY(channel_lives == 0)) {
                                       channel_lives_map_.erase(channel_id);
                                   }
                                   else if (channel_lives_map_[channel_id] == 1) {
                                       channel_lives_map_.erase(channel_id);
                                       channel_event_map_[channel_id] |= EVENT_TIMEOVER;
                                   }
                                   else {
                                       channel_lives_map_[channel_id]--;
                                   }
                               }
            );
        }
    }

    inline void start() noexcept {
        loop();
    }

    inline void stop() noexcept {
        quit_ = true;
    }

private:
    enum class INIT_STATUS {
        INIT,
        SUCCESS,
        ERROR
    };

    inline void init() {
        if (create_epoll_fd() < 0 || create_timer_fd() < 0 || add_timer_channel() < 0) {
            init_status_ = INIT_STATUS::ERROR;
        }
        else {
            init_status_ = INIT_STATUS::SUCCESS;
            quit_ = false;
        }
    }

    void loop() noexcept;

    void handle_cb() noexcept;

    inline int create_epoll_fd() noexcept {
        return epoll_ = epoll_create1(EPOLL_CLOEXEC);
    }

    int create_timer_fd() noexcept;

    int add_timer_channel();

    inline bool unlawful_fd(int fd) const noexcept {
        return (fd < 0 || fd == epoll_ || fd == timer_);
    }


    void add_task_on_loop_recursive(size_t seconds, void *user_arg,
                                    const std::function<void(EventLoopPtr &, void *user_arg, bool *again)> &cb) {
        task_wheel_.regist(seconds,
                           [this, seconds, cb, user_arg]() {
                               bool again = false;
                               cb(this, user_arg, &again);
                               if (again) {
                                   add_task_on_loop_recursive(seconds, user_arg, std::move(cb));
                               }
                           });
    }

    void add_task_on_channel_recursive(ChannelId channel_id, size_t seconds, void *user_arg,
                                       const std::function<void(EventLoopPtr &, ChannelPtr &, void *user_arg,
                                                                bool *again)> &cb) {
        if (channel_map_.find(channel_id) != channel_map_.end()) {
            task_wheel_.regist(seconds,
                               [this, channel_id, seconds, cb, user_arg]() {
                                   if (channel_map_.find(channel_id) != channel_map_.end()) {
                                       bool again = false;
                                       cb(this, channel_map_[channel_id], user_arg, &again);
                                       if (again) {
                                           add_task_on_channel_recursive(channel_id, seconds, user_arg, std::move(cb));
                                       }
                                   }
                               });
        }
    }

public:
    Context context;
    ContextDeleter context_deleter;
private:
    INIT_STATUS init_status_;
    int epoll_;
    int timer_;
    bool quit_;
    std::unordered_map<ChannelId, ChannelPtr> channel_map_;
    std::unordered_map<ChannelId, ChannelEvent> channel_event_map_;
    std::unordered_map<ChannelId, size_t> channel_lives_map_;
    TaskWheel task_wheel_;
    epoll_event reg_event_;
    static ChannelPtr null_channel_ptr;
};

#endif // EVENTLOOP_EVENTLOOP_H
