#include "../include/hpsa.h"

void webchild(int listenfd)
{
    int connfd, nfds, epfd, ret;
    struct epoll_event *events;
    struct epoll_event ev;
    struct sockaddr cli_addr; 
    socklen_t len;
    clients_list clients;

    if ((epfd = epoll_create(CONN_PER_PROCESS)) < 0) {
        eret("epoll_create error");
        exit(errno);
    }
    ev.data.fd = listenfd;
    ev.events  = EPOLLET | EPOLLIN;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
        eret("epoll_ctl error");
        exit(errno);
    }

    len = sizeof(struct sockaddr);
    signal(SIGPIPE, SIG_IGN);
    events = alloca(sizeof(struct epoll_event) * CONN_PER_PROCESS);
    memset(events, 0, sizeof(struct epoll_event) * CONN_PER_PROCESS);
    clients_init(&clients);

    for ( ; ; ) {
        clients_timeout(&clients, epfd);
        nfds = epoll_wait(epfd, events, CONN_PER_PROCESS, -1);
        if (nfds == -1) {
            eret("epoll_wait error");
            exit(errno);
        }
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listenfd) { /* Accept events. */
                if ((connfd = accept_e(listenfd, &cli_addr, &len)) < 0) {
                    debug("%d\n", clients.now);
                    exit(errno);
                }
                if (clients_add(&clients, connfd) < 0 || event_add(epfd, connfd, EPOLLIN) < 0) {
                    close(connfd);
                    continue;
                }
                printf("%d %d %s %d\n", getpid(), connfd, sock_ntop(&cli_addr), clients.now);
            } else if (events[i].events & EPOLLIN) { /* Read events. */
                client_t *cli = clients_find(&clients, events[i].data.fd);
                if ((ret = handle_read(cli)) == READ_FAILURE) {
                    clients_del(epfd, &clients, events[i].data.fd);
                    continue;
                }
                if ((ret = http_parse(events[i].data.fd, cli)) == HTTP_PARSE_OK) {
                    event_mod(epfd, events[i].data.fd, EPOLLOUT);
                } else if (ret == FILE_TOO_LARGE) {
                    /* Do nothing. */
                } else {
                    clients_del(epfd, &clients, events[i].data.fd);
                    continue;
                }
                clients_update_timeout(&clients, events[i].data.fd);
            } else if (events[i].events & EPOLLOUT) { /* Write events. */
                client_t *cli = clients_find(&clients, events[i].data.fd);
                if ((ret = handle_write(cli)) == WRITE_FAILURE) {
                    clients_del(epfd, &clients, events[i].data.fd);
                    continue;
                } else if (ret == WRITE_AGAIN) {
                    continue;
                }
                event_mod(epfd, events[i].data.fd, EPOLLIN);
                clients_update_timeout(&clients, events[i].data.fd);
            } else {
                clients_del(epfd, &clients, events[i].data.fd);
            }
        } /* handle epoll task */
    } /* main loop */
}
