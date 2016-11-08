#include "../include/hpsa.h"

int event_mod(int epfd, int fd, int op)
{
    struct epoll_event ev;

    ev.data.fd = fd;
    ev.events  = op | EPOLLET;
    return epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}

/* if op == 0, then ev.events = EPOLLIN | EPOLLET */
int event_add(int epfd, int fd, int op)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events  = (op ? op : EPOLLIN) | EPOLLET;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

int event_del(int epfd, int fd)
{
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}
