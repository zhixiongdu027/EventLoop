// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_CHANNELFORWARD_H
#define EVENTLOOP_CHANNELFORWARD_H

#include <stdint.h>
#include <memory>
#include <functional>


class EventLoop;
typedef EventLoop* const EventLoopPtr;

class StreamBuffer;

typedef std::unique_ptr<StreamBuffer> StreamBufferPtr;

class Channel;
typedef uint32_t ChannelEvent;
typedef uint32_t ChannelId;
typedef std::unique_ptr<Channel> ChannelPtr;
typedef std::function<void(EventLoopPtr&, ChannelPtr&, ChannelEvent)> ChannelCallback;

const ChannelEvent TODO_REGO = 1lu << 30;
const ChannelEvent TODO_SHUTDOWN = 1lu << 29;
const ChannelEvent TODO_ERASE = 1lu << 28;
const ChannelEvent TODO_OUTPUT = 1lu << 27;

const ChannelEvent EVENT_IN = 1lu << 15;
const ChannelEvent EVENT_HUP = 1lu << 14;
const ChannelEvent EVENT_CONNECTERR = 1lu << 13;
const ChannelEvent EVENT_SENDERR = 1lu << 12;
const ChannelEvent EVENT_TIMEOVER = 1lu << 11;
const ChannelEvent EVENT_EPOLLERR = 1lu << 10;

#endif //EVENTLOOP_CHANNELFORWARD_H
