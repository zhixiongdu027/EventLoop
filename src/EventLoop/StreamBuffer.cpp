// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include "StreamBuffer.h"

size_t StreamBuffer::DefaultPrependable = 12;
size_t StreamBuffer::DefaultCapacity = 1024;

void StreamBuffer::ensure_append_size_with_memory_operator(size_t len) {
  size_t moveable = (read_pos_ > DefaultPrependable ? read_pos_ - DefaultPrependable : 0);
  if (moveable + writeable() >= len) {
    ::memmove(memory_ + DefaultPrependable, peek(), readable());
    read_pos_ -= moveable;
    write_pos_ -= moveable;
    return;
  }

  size_t new_capacity = (len + capacity_) * 2;
  char *new_memory_ = new char[new_capacity];
  ::memcpy(new_memory_ + DefaultPrependable, peek(), readable());
  delete[] memory_;

  memory_ = new_memory_;
  capacity_ = new_capacity;

  ssize_t diff_len = read_pos_ - DefaultPrependable; //may be less than zero

  read_pos_ -= diff_len;
  write_pos_ -= diff_len;
  return;
}

void StreamBuffer::insert(size_t position, const void *data, size_t len) {
  assert(position <= readable());
  if (position == readable()) {
    append(data, len);
  }
  else if (len <= writeable()) {
    const char *move_src = peek(position);
    size_t move_len = readable() - position;
    char *move_dst = peek(position) + len;
    ::memmove(move_dst, move_src, move_len);
    ::memcpy(peek(position), data, len);
    write_pos_ += len;
  }
  else if (len <= prependable()) {
    const char *move_src = peek();
    size_t move_len = position;
    char *move_dst = peek() - len;
    ::memmove(move_dst, move_src, move_len);
    read_pos_ -= len;
    ::memcpy(peek(position), data, len);
  }
  else if (len < prependable() + writeable()) {
    const char *right_move_src = peek(position);
    size_t right_move_len = readable() - position;
    char *right_move_dst = peek(position) + writeable();

    ::memmove(right_move_dst, right_move_src, right_move_len);

    const char *left_move_src = peek();
    size_t left_move_len = position;
    char *left_move_dst = peek() - (len - writeable());

    ::memmove(left_move_dst, left_move_src, left_move_len);

    read_pos_ -= (len - writeable());
    ::memcpy(peek(position), data, len);
    write_pos_ = capacity();
  }
  else {
    size_t new_capacity = (len + capacity_) * 2;
    char *new_memory_ = new char[new_capacity];
    ::memcpy(new_memory_ + DefaultPrependable, peek(), position);
    ::memcpy(new_memory_ + DefaultPrependable + position, data, len);
    ::memcpy(new_memory_ + DefaultPrependable + position + len, peek(position), readable() - position);
    delete[] memory_;
    memory_ = new_memory_;
    capacity_ = new_capacity;
    write_pos_ = DefaultPrependable + readable() + len;
    read_pos_ = DefaultPrependable;
  }
}

void StreamBuffer::replace(size_t position, size_t replace_len, const void *data, size_t len) {
  assert(position + replace_len <= readable());
  if (replace_len == len) {
    ::memcpy(peek(position), data, len);
  }
  else if (replace_len > len) {
    ::memcpy(peek(position), data, len);
    discard(position + len, replace_len - len);
  }
  else if (replace_len < len) {
    ::memcpy(peek(position), data, replace_len);
    insert(position + replace_len, (const char *) data + replace_len, len - replace_len);
  }
}

StreamBuffer::IO_RES StreamBuffer::read_fd(int fd, ssize_t *actual_read) noexcept {
  static const size_t EXTRA_BUFF_SIZE = 65535;
  char extra_buff[EXTRA_BUFF_SIZE];
  struct iovec vec[2];
  vec[0].iov_base = memory() + write_pos_;
  vec[0].iov_len = writeable();
  vec[1].iov_base = &extra_buff[0];
  vec[1].iov_len = EXTRA_BUFF_SIZE;

  ssize_t last_read;
  ssize_t *last_read_ptr = &last_read;
  if (actual_read != nullptr) {
    last_read_ptr = actual_read;
  }

  *last_read_ptr = ::readv(fd, vec, 2);

  if (*last_read_ptr < 0) {
    if (errno == EINTR)
      return read_fd(fd, actual_read);
    else if (errno == EWOULDBLOCK || errno == EAGAIN)
      return AGAIN;
    else
      return OTHER;
  }
  if (*last_read_ptr == 0) {
    return ENDOF;
  }
  else if (static_cast<size_t>(*last_read_ptr) <= writeable()) {
    write_pos_ += *last_read_ptr;
  }
  else {
    size_t len = *last_read_ptr - writeable();
    write_pos_ = capacity_;
    append(extra_buff, len);
  }
  return OK;
}

StreamBuffer::IO_RES StreamBuffer::write_fd(int fd, ssize_t *actual_write) noexcept {
  ssize_t last_write;
  ssize_t *last_write_ptr = &last_write;
  if (actual_write != nullptr) {
    last_write_ptr = actual_write;
  }

  *last_write_ptr = ::write(fd, peek(), readable());
  if (*last_write_ptr < 0) {
    if (errno == EINTR)
      return write_fd(fd, peek(), readable(), actual_write);
    else {
      return errno == EAGAIN ? AGAIN : OTHER;
    }
  }
  assert(readable() >= static_cast<size_t>(*last_write_ptr));
  discard(static_cast<size_t>(*last_write_ptr));
  return static_cast<size_t>(*last_write_ptr) == readable() ? OK : AGAIN;
}

StreamBuffer::IO_RES StreamBuffer::write_fd(int fd, const void *data, size_t len, ssize_t *actual_write) noexcept {
  assert(data != nullptr);
  iovec vec[2];
  vec[0].iov_base = peek();
  vec[0].iov_len = readable();
  vec[1].iov_base = (void *) data;
  vec[1].iov_len = len;

  ssize_t last_write;
  ssize_t *last_write_ptr = &last_write;
  if (actual_write != nullptr) {
    last_write_ptr = actual_write;
  }

  do {
    *last_write_ptr = ::writev(fd, &vec[0], 2);
    if (*last_write_ptr < 0) {
      if (errno == EINTR) {
        continue;
      }
      else {
        append(data, len);
        return errno == EAGAIN ? AGAIN : OTHER;
      }
    }
  } while (false);

  if (static_cast<size_t>(*last_write_ptr) == readable() + len) {
    return OK;
  }

  if (static_cast<size_t>(*last_write_ptr) <= readable()) {
    discard((size_t) *last_write_ptr);
    append(data, len);
  }
  else {
    size_t data_write_len = static_cast<size_t>(*last_write_ptr) - readable();
    discard_all();
    append(static_cast<const char *>(data) + data_write_len, len - data_write_len);
  }
  return AGAIN;
}

template<>
void std::swap(StreamBuffer &lhs, StreamBuffer &rhs) {
  lhs.swap(rhs);
}
