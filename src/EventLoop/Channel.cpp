// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#include "Channel.h"

void Channel::send(const void *data, size_t len, bool is_noblock_fd) noexcept {
  assert(data != nullptr && connected_);
  if (is_noblock_fd) {
    StreamBuffer::IO_RES io_res = writeBuffer_.write_fd(fd(), data, len, nullptr);
    if (io_res == StreamBuffer::OK) {
      return;
    }
    else if (io_res == StreamBuffer::AGAIN) {
      channel_todo_map_[id()] |= TODO_REGO;
    }
    else {
      channel_event_map_[id()] |= EVENT_SENDERR;
    }
  }
  else {
    writeBuffer_.append(data, len);
    channel_todo_map_[id()] |= TODO_REGO;
  }
}

void Channel::send_to_socket(const void *data, size_t len, const sockaddr *addr, socklen_t *addr_len) noexcept {
  assert(data != nullptr);
  if (!connected_) {
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
  msg.msg_name = (void *) addr;
  msg.msg_namelen = addr_len == nullptr ? 0 : *addr_len;
  msg.msg_iov = &vec[0];
  msg.msg_iovlen = 2;

  ssize_t last_write;

  do {
    last_write = ::sendmsg(fd(), &msg, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (last_write < 0) {
      if (errno == EINTR) {
        continue;
      }
      else if (errno == EWOULDBLOCK || errno == EAGAIN) {
        writeBuffer_.append(data, len);
        channel_todo_map_[id()] |= TODO_REGO;
      }
      else {
        channel_event_map_[id()] |= EVENT_SENDERR;
      }
    }
  } while (false);

  if (writeBuffer_.readable() + len == last_write) {
    writeBuffer_.discard_all();
    // nothing;     don't call back when send_to_socket success;
    return;
  }
  else {
    channel_todo_map_[id()] |= TODO_REGO;
    if (writeBuffer_.readable() >= last_write) {
      writeBuffer_.discard(static_cast<size_t>(last_write));
      writeBuffer_.append(data, len);
    }
    else {
      size_t used_data = last_write - writeBuffer_.readable();
      writeBuffer_.discard_all();
      writeBuffer_.append(static_cast<const char *>(data) + used_data, len - used_data);
    }
  }
}
