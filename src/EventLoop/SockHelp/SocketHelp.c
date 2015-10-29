#include "SocketHelp.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int tcp_connect_overtime(const char *host, int port, int timeout) {
    struct addrinfo hints, *res;

    memset(&hints, 0x00, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (port > 65535) return -1;

    char port_str[6];
    sprintf(port_str, "%d", port);
    int result = getaddrinfo(host, port_str, &hints, &res);
    if (result != 0) {
        return -1;
    }

    int socket_fd = socket(res->ai_family, SOCK_STREAM, res->ai_protocol);
    if (socket_fd < 0) {
        freeaddrinfo(res);
        return -1;
    }

    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    result = connect(socket_fd, res->ai_addr, res->ai_addrlen);
    if (result == 0) {
        result = 1;
    }
    else if (result < 0 && errno != EINPROGRESS) {
        result = -1;
    }
    else {
        do {
            /* set connection timeout */
            struct timeval tv;
            fd_set rfds, wfds;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;

            FD_ZERO(&wfds);
            FD_SET(socket_fd, &wfds);
            rfds = wfds;

            result = select(socket_fd + 1, &rfds, &wfds, NULL, &tv);
            if (result == 0) {
                result = -1;
                break;
            }
            if (result < 0 && errno != EINTR) {
                result = -1;
                break;
            }
            else if (result > 0) {
                socklen_t optlen = sizeof(int);
                int optval;
                if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (void *) (&optval), &optlen) < 0) {
                    result = -1;
                    break;
                }
                if (optval != 0) {
                    result = -1;
                    break;
                }
                result = 1;
                break;
            }
            else {
                result = -1;
                break;
            }
        }
        while (1);
    }
    freeaddrinfo(res);
    return (result == 1 ? socket_fd : result);
}

int tcp_connect(const char *host, unsigned short port) {

    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    char server[6] = {'\0'};
    sprintf(server, "%d", port);
    if (getaddrinfo(host, server, &hints, &res) != 0) {
        return -1;
    }

    ressave = res;
    int sock_fd;
    do {
        sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock_fd < 0) {
            continue;
        }

        if (connect(sock_fd, res->ai_addr, res->ai_addrlen) == 0) {
            break;
        }

        close(sock_fd);
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        return -1;
    }

    freeaddrinfo(ressave);
    return sock_fd;
}

int create_tcp_listen(unsigned short port, int reuse) {
    struct sockaddr_in addr;
    memset(&addr, 0x00, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        return -1;
    }
    if(set_no_block(socket_fd)<0)
    {
        return -1;
    }

    int res;
    if (reuse != 0) {
        const int _reuse = 1;
        res = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &_reuse, sizeof(int));
        if (res < 0) {
            return -1;
        }
    }

    res = bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr));
    if (res < 0) {
        return -1;
    }

    res = listen(socket_fd, 10);
    if (res < 0) {
        return -1;
    }
    return socket_fd;
}

int set_no_block(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}