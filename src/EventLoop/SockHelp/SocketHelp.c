#include "SocketHelp.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

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
    int sock_fd = -1;

    for (; res != NULL; res = res->ai_next) {
        sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock_fd < 0) {
            continue;
        }
        if (connect(sock_fd, res->ai_addr, res->ai_addrlen) == 0) {
            break;
        }
        else {
            close(sock_fd);
            sock_fd = -1;
        }
    };

    freeaddrinfo(ressave);
    return sock_fd;
}

int create_tcp_listen(unsigned short port, int reuse) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        return -1;
    }
    if (fcntl(socket_fd, F_SETFL, O_NONBLOCK)) {
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

    struct sockaddr_in addr;
    memset(&addr, 0x00, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

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

NONBLOCK_CONNECT_STATUS tcp_nonblock_connect(const char *host, unsigned short port, int *sock_fd) {
    assert(sock_fd != NULL);
    struct addrinfo hints, *res, *ressave;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    char server[6] = {'\0'};
    sprintf(server, "%d", port);
    if (getaddrinfo(host, server, &hints, &res) != 0) {
        return NONBLOCK_CONNECT_ERROR;
    }
    ressave = res;

    NONBLOCK_CONNECT_STATUS return_value = NONBLOCK_CONNECT_ERROR;
    if (res != NULL) {
        *sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (*sock_fd < 0 || fcntl(*sock_fd, F_SETFL, O_NONBLOCK) < 0) {
            close(*sock_fd);
        }
        else {
            int connect_res = connect(*sock_fd, res->ai_addr, res->ai_addrlen);
            if (connect_res == -1 && errno == EINPROGRESS) {
                return_value = NONBLOCK_CONNECT_INPROGRESS;
            }
            else if (connect_res == 0) {
                return_value = NONBLOCK_CONNECT_OK;
            }
            else {
                close(*sock_fd);
                *sock_fd = -1;
                return_value = NONBLOCK_CONNECT_ERROR;
            }
        }
    }

    freeaddrinfo(ressave);
    return return_value;
}

int create_udp_listen(unsigned short port, int reuse) {
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        return -1;
    }

    if (reuse != 0) {
        const int _reuse = 1;
        if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &_reuse, sizeof(int)) < 0) {
            close(sock_fd);
            return -1;
        }
    }

    struct sockaddr_in addr;
    memset(&addr, 0x00, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htobe32(INADDR_ANY);
    addr.sin_port = htobe16(port);

    return bind(sock_fd, (const struct sockaddr *) &addr, sizeof(addr));
}

int udp_connect(const char *host, unsigned short port) {

    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV;

    char server[6] = {'\0'};
    sprintf(server, "%d", port);
    if (getaddrinfo(host, server, &hints, &res) != 0) {
        return -1;
    }

    ressave = res;

    int sock_fd = -1;
    for (; res != NULL; res = res->ai_next) {
        sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock_fd < 0) {
            continue;
        }
        if (connect(sock_fd, res->ai_addr, res->ai_addrlen) == 0) {
            break;
        }
        else {
            close(sock_fd);
            sock_fd = -1;
        }
    };

    freeaddrinfo(ressave);
    return sock_fd;
}
