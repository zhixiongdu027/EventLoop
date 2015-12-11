//
// Created by adugeek on 15-7-1.
//

#include <net/if.h>
#include <cstdio>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/wait.h>
#include <iostream>
#include "ProxyServer.h"
#include "EventLoop/tool/SocketHelp.h"

std::unordered_set<pid_t> proxy_process_set;
int proxy_listen_socket;

static pid_t process_proxy() {
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }
    else if (pid == 0) {
        ProxyServer proxyServer(proxy_listen_socket);
        proxyServer.run();
        return -1;
    }
    return pid;
};

static int create_child() {
    ssize_t need_fork;
    if (proxy_process_set.size() < define::proxy_process_size) {
        need_fork = define::proxy_process_size - proxy_process_set.size();
        for (int i = 0; i < need_fork; ++i) {
            pid_t new_pid = process_proxy();
            if (need_fork < 0) {
                return -1;
            }
            if (need_fork > 0) {
                proxy_process_set.insert(new_pid);
            }
            usleep(10);
        }
    }
    return 0;
}

static int create_signalfd() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGCHLD);
    return sigprocmask(SIG_BLOCK, &mask, NULL) < 0 ? -1 : signalfd(-1, &mask, SFD_CLOEXEC);
}

static void cleanup() {
    for (auto &item :proxy_process_set) {
        kill(item, -9);
        usleep(10);
    }
    proxy_process_set.clear();

    int stat;
    while (waitpid(-1, &stat, WNOHANG) > 0);

    close(proxy_listen_socket);
    proxy_listen_socket = -1;
}

static void signal_event_cb(EventLoopPtr &loop_ptr, ChannelPtr &channel_Ptr, uint32_t events) {
    if (events != EVENT_IN) {
        loop_ptr->stop();
        return;
    }

    signalfd_siginfo fdsi;
    if (read(channel_Ptr->fd(), &fdsi, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo)) {
        loop_ptr->stop();
        return;
    }

    if (fdsi.ssi_signo == SIGCHLD) {
        int stat;
        pid_t exit_pid;
        while ((exit_pid = waitpid(-1, &stat, WNOHANG)) > 0) {
            if (proxy_process_set.find(exit_pid) != proxy_process_set.end()) {
                proxy_process_set.erase(exit_pid);
            }
            usleep(10);
        }
    }
}

static void timed_task(EventLoopPtr &loop_ptr, void *, bool *again) {
    *again = true;
    if (create_child() < 0) {
        loop_ptr->stop();
    }
}

int main() {
    while (true) {
        do {
            proxy_listen_socket = create_tcp_listen(80, 1);
            if (proxy_listen_socket < 0) {
                std::cout << "create tcp listen on 80 port error " << std::endl;
                break;
            }

            int signal_fd = create_signalfd();

            EventLoop loop;
            if (loop.add_channel(signal_fd, false, false, -1, signal_event_cb) == nullptr) {
                break;
            }

            loop.add_task_on_loop(true, 30, nullptr, timed_task);
            loop.start();
        } while (false);
        cleanup();
        sleep(45);
    }
}