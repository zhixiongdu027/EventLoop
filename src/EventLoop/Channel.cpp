// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#include <errno.h>
#include "Channel.h"

int Channel::read() noexcept{
    read_begin:
    ssize_t last_read = readBuffer_.read_some(fd());
    if (last_read > 0) {
        return 1;
    }
    else if (last_read == 0) {
        return 0;
    }
    else {
        if (errno == EINTR) {
            goto read_begin;
        }
        else if (errno == EAGAIN) {
            return 1;
        }
        else {
            return -1;
        }
    }
}

void Channel::send() noexcept {
    if (!writeBuffer_.empty()) {
        if (!is_socket_) {
            channel_event_map_[id()] |= TODO_REGO;
        }
        else {
            read_begin:
            ssize_t last_write = ::send(fd(), writeBuffer_.peek(), writeBuffer_.readable(), MSG_DONTWAIT | MSG_NOSIGNAL);
            if (last_write == writeBuffer_.readable()) {
                writeBuffer_.discard_all();
            }
            else if (last_write >= 0) {
                writeBuffer_.discard(last_write);
                channel_event_map_[id()] |= TODO_REGO;
            }
            else {
                if (errno == EINTR) {
                    goto read_begin;
                }
                else if (errno == EAGAIN) {
                    channel_event_map_[id()] |= TODO_REGO;
                }
                else {
                    channel_event_map_[id()] |= EVENT_SENDERR;
                }
            }
        }
    }
}

void Channel::send_to_normal(const void *data, size_t len) noexcept {
    assert(data != nullptr);
    if (is_nonblock_)
    {
        read_begin_label:
        ssize_t last_write = writeBuffer_.write(fd(), data, len);
        if (last_write == len) {
            //nothing ;
        }
        else if (last_write >= 0) {
            writeBuffer_.append(static_cast<const char *>(data) + last_write, len - last_write);
            channel_event_map_[id()] |= TODO_REGO;
        }
        else {
            if (errno == EINTR) {
                goto read_begin_label;
            }
            else if (errno == EAGAIN) {
                channel_event_map_[id()] |= TODO_REGO;
            }
            else {
                channel_event_map_[id()] |= EVENT_SENDERR;
            }
        }
    }
    else
    {
        writeBuffer_.append(data, len);
        channel_event_map_[id()] |= TODO_REGO;
    }
}

void Channel::send_to_socket(const void *data, size_t len) noexcept {
    assert(data != nullptr);
    if (!is_connected_) {
        writeBuffer_.append(data, len);
        return;
    }
    iovec vec[2];
    vec[0].iov_base = writeBuffer_.peek();
    vec[0].iov_len = writeBuffer_.readable();
    vec[1].iov_base = (void *) data;
    vec[1].iov_len = len;

    msghdr msg;
    memset(&msg, 0, sizeof(msghdr));
    msg.msg_iov = &vec[0];
    msg.msg_iovlen = 2;

    read_begin:
    ssize_t last_write = sendmsg(fd(), &msg, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (last_write == writeBuffer_.readable() + len) {
        writeBuffer_.discard_all();
    }
    else if (last_write >= writeBuffer_.readable()) {
        size_t used_data = last_write - writeBuffer_.readable();
        writeBuffer_.discard_all();
        writeBuffer_.append(static_cast<const char *>(data) + used_data, len - used_data);
        channel_event_map_[id()] |= TODO_REGO;
    }
    else if (last_write >= 0) {
        writeBuffer_.discard(last_write);
        writeBuffer_.append(data, len);
        channel_event_map_[id()] |= TODO_REGO;
    }
    else {
        if (errno == EINTR) {
            goto read_begin;
        }
        else if (errno == EAGAIN) {
            channel_event_map_[id()] |= TODO_REGO;
        }
        else {
            channel_event_map_[id()] |= EVENT_SENDERR;
        }
    }
}