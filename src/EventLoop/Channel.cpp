// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#include <errno.h>
#include "Channel.h"

int Channel::read() noexcept{
    read_begin_label:
    ssize_t read_res = read_buffer_->read_some(fd());
    if (read_res > 0) {
        return 1;
    }
    else if (read_res == 0) {
        return 0;
    }
    else {
        if (errno == EINTR) {
            goto read_begin_label;
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
    if (!write_buffer_->empty()) {
        if (!is_socket_) {
            channel_event_map_[id()] |= TODO_REGO;
        }
        else {
            write_begin_label:
            ssize_t write_res = ::send(fd(), write_buffer_->peek(), write_buffer_->peek_able(),
                                       MSG_DONTWAIT | MSG_NOSIGNAL);
            if (write_res == write_buffer_->peek_able()) {
                write_buffer_->discard_all();
            }
            else if (write_res >= 0) {
                write_buffer_->discard(write_res);
                channel_event_map_[id()] |= TODO_REGO;
            }
            else {
                if (errno == EINTR) {
                    goto write_begin_label;
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
        write_begin_label:
        ssize_t write_res = write_buffer_->write(fd(), data, len);
        if (write_res == len) {
            //nothing ;
        }
        else if (write_res >= 0) {
            write_buffer_->append(static_cast<const char *>(data) + write_res, len - write_res);
            channel_event_map_[id()] |= TODO_REGO;
        }
        else {
            if (errno == EINTR) {
                goto write_begin_label;
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
        write_buffer_->append(data, len);
        channel_event_map_[id()] |= TODO_REGO;
    }
}

void Channel::send_to_socket(const void *data, size_t len) noexcept {
    assert(data != nullptr);
    if (!is_connected_) {
        write_buffer_->append(data, len);
        return;
    }
    iovec vec[2];
    vec[0].iov_base = write_buffer_->peek();
    vec[0].iov_len = write_buffer_->peek_able();
    vec[1].iov_base = (void *) data;
    vec[1].iov_len = len;

    msghdr msg;
    memset(&msg, 0, sizeof(msghdr));
    msg.msg_iov = &vec[0];
    msg.msg_iovlen = 2;

    write_begin_able:
    ssize_t write_res = sendmsg(fd(), &msg, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (write_res == write_buffer_->peek_able() + len) {
        write_buffer_->discard_all();
    }
    else if (write_res >= write_buffer_->peek_able()) {
        size_t used_data = write_res - write_buffer_->peek_able();
        write_buffer_->discard_all();
        write_buffer_->append(static_cast<const char *>(data) + used_data, len - used_data);
        channel_event_map_[id()] |= TODO_REGO;
    }
    else if (write_res >= 0) {
        write_buffer_->discard(write_res);
        write_buffer_->append(data, len);
        channel_event_map_[id()] |= TODO_REGO;
    }
    else {
        if (errno == EINTR) {
            goto write_begin_able;
        }
        else if (errno == EAGAIN) {
            channel_event_map_[id()] |= TODO_REGO;
        }
        else {
            channel_event_map_[id()] |= EVENT_SENDERR;
        }
    }
}