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

class EventLoop;
typedef EventLoop* const EventLoopPtr;

class Channel;
typedef uint32_t ChannelEvent;
typedef uint32_t ChannelId;
typedef std::unique_ptr<Channel> ChannelPtr;
typedef std::function<void(EventLoopPtr&, ChannelPtr&, ChannelEvent)> ChannelCallback;

const ChannelEvent TODO_REGO = 1lu << 31;
const ChannelEvent TODO_SHUTDOWN = 1lu << 30;
const ChannelEvent TODO_ERASE = 1lu << 29;
const ChannelEvent TODO_OUTPUT = 1lu << 28;

const ChannelEvent EVENT_IN = 1lu << 15;
const ChannelEvent EVENT_HUP = 1lu << 14;
const ChannelEvent EVENT_TIMEOVER = 1lu << 13;
const ChannelEvent EVENT_EPOLL_ERR = 1lu << 12;
const ChannelEvent EVENT_CONNECT_ERR = 1lu << 11;
const ChannelEvent EVENT_CONNECT_TIMEOVER = 1lu << 10;
const ChannelEvent EVENT_SEND_ERR = 1lu << 9;

typedef union {
    void *ptr;
    int32_t i32;
    int64_t i64;
    uint32_t u32;
    uint64_t u64;
} Context;

typedef std::function<void(void *)> ContextDeleter;

#endif //EVENTLOOP_FORWARD_H
