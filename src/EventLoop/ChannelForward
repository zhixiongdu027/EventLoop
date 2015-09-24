//
// Created by yang on 6/30/15.
//

#ifndef EVENTLOOP_CHANNELFORWARD_H
#define EVENTLOOP_CHANNELFORWARD_H

#include <stdint.h>
#include <memory>
#include <functional>

class EventLoop;
class Channel;

typedef uint32_t ChannelEvent;
typedef uint32_t ChannelTodo;
typedef uint32_t ChannelId;
typedef std::shared_ptr<Channel> ChannelPtr;
typedef std::function<void(EventLoop *, ChannelPtr &, ChannelEvent)> ChannelCallback;

const ChannelTodo TODO_REGO = 1lu << 30;
const ChannelTodo TODO_SHUTDOWN = 1lu << 29;
const ChannelTodo TODO_ERASE = 1lu << 28;
const ChannelTodo TODO_OUTPUT = 1lu << 27;

const ChannelEvent EVENT_IN = 1lu << 15;
const ChannelEvent EVENT_HUP = 1lu << 14;
const ChannelEvent EVENT_CONNECTERR = 1lu << 13;
const ChannelEvent EVENT_SENDERR = 1lu << 12;
const ChannelEvent EVENT_TIMEOVER = 1lu << 11;
const ChannelEvent EVENT_EPOLLERR = 1lu << 10;

#endif //EVENTLOOP_CHANNELFORWARD_H
