/*
 * File:   sockethelp.h
 * Author: adugeek
 *
 * Created on November 4, 2014, 2:29 PM
 */

#ifndef SOCKETHELP_H
#define SOCKETHELP_H

#ifdef    __cplusplus
extern "C"
{
#endif

#include <netdb.h>
int tcp_connect(const char *host, unsigned short port);

typedef enum {
    NONBLOCK_CONNECT_OK,
    NONBLOCK_CONNECT_INPROGRESS,
    NONBLOCK_CONNECT_ERROR
} NONBLOCK_CONNECT_STATUS;

NONBLOCK_CONNECT_STATUS tcp_nonblock_connect(const char *host, unsigned short port, int *sock_fd);

int create_tcp_listen(unsigned short port, int reuse);

int set_no_block(int fd);

#ifdef    __cplusplus
}
#endif

#endif	/* SOCKETHELP_H */

