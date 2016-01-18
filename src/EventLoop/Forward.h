// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_FORWARD_H
#define EVENTLOOP_FORWARD_H

#include <errno.h>
#include <stdint.h>
#include <memory>
#include <functional>
#include "tool/Likely.h"
#include "tool/Copyable.h"

class EventLoop;

typedef EventLoop *const EventLoopPtr;

class Channel;

typedef uint32_t ChannelEvent;
typedef uint32_t ChannelId;
typedef std::unique_ptr<Channel> ChannelPtr;
typedef std::function<void(EventLoopPtr &, ChannelPtr &, ChannelEvent)> ChannelCallback;
typedef std::function<void(EventLoopPtr &, ChannelPtr &, void *user_arg, bool *again)> ChannelTask;
typedef std::function<void(EventLoopPtr &, void *user_arg, bool *again)> EventLoopTask;

constexpr ChannelEvent TODO_REGO = 1lu << 31;
constexpr ChannelEvent TODO_SHUTDOWN = 1lu << 30;
constexpr ChannelEvent TODO_ERASE = 1lu << 29;
constexpr ChannelEvent TODO_OUTPUT = 1lu << 28;

constexpr ChannelEvent EVENT_IN = 1lu << 15;
constexpr ChannelEvent EVENT_HUP = 1lu << 14;
constexpr ChannelEvent EVENT_TIMEOVER = 1lu << 13;
constexpr ChannelEvent EVENT_EPOLL_ERR = 1lu << 12;
constexpr ChannelEvent EVENT_CONNECT_ERR = 1lu << 11;
constexpr ChannelEvent EVENT_CONNECT_TIMEOVER = 1lu << 10;
constexpr ChannelEvent EVENT_SEND_ERR = 1lu << 9;

typedef union {
    void *ptr;
    int32_t i32;
    int64_t i64;
    uint32_t u32;
    uint64_t u64;
} Context;

typedef std::function<void(void *)> ContextDeleter;

#endif //EVENTLOOP_FORWARD_H
