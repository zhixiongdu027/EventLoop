// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: adugeek

#ifndef EVENTLOOP_TOOL_SOCKETHELP_H
#define EVENTLOOP_TOOL_SOCKETHELP_H

#ifdef    __cplusplus
extern "C"
{
#endif

#include <netdb.h>
#include "ExecuteState.h"

int tcp_connect(const char *host, unsigned short port);

int udp_connect(const char *host, unsigned short port);

ExecuteState tcp_nonblock_connect(const char *host, unsigned short port, int *sock_fd);

int create_tcp_listen(unsigned short port, int reuse);

int create_udp_listen(unsigned short port, int reuse);

int set_no_block(int fd);

#ifdef    __cplusplus
}
#endif

#endif	/* EVENTLOOP_TOOL_SOCKETHELP_H */

