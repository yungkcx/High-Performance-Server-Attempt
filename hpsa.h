#ifndef HASP_H__
#	define HASP_H__

#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>

#include "lib/clients.h"
#include "lib/http_parse.h"
#include "lib/config.h"

#define VERSION           "1.1"
#define MAXLINE           4096
#define TIMEBUF           40
#define CONN_PER_PROCESS  200

#define UNUSED(v)            (void)(v)
#define max(a, b) ({\
		typeof(a) _max1 = (a);\
		typeof(b) _max2 = (b);\
		_max1 > _max2 ? _max1 : _max2;})
#define STRCMP(a, R, b)          (strcmp(a, b) R 0)
#define STRNCMP(a, R, b, n)      (strncmp(a, b, n) R 0)
#define STRCASECMP(a, R, b)      (strcasecmp(a, b) R 0)
#define STRNCASECMP(a, R, b, n)  (strncasecmp(a, b, n) R 0)
#define debug(M, ...)        fprintf(stderr, "DEBUG %s (in function '%s'):%d: " M "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

enum {
    WRITE_OK,
    WRITE_FAILURE,
    READ_OK,
    READ_FAILURE
};

const char *sock_ntop(const struct sockaddr *sa);
int tcp_connect(const char *host, const char *port);
int tcp_listen(const char *host, const char *port, socklen_t *addrlenp, int backlog);
int set_fl(int fd, int flags);
int clr_fl(int fd, int flags);
int accept_e(int sockfd, struct sockaddr *addr, socklen_t *addrlenp);
ssize_t writen(int fd, const void *buf, size_t n);
ssize_t readn(int fd, void *buf, size_t n);

void eret(const char *fmt, ...);
void esys(const char *fmt, ...);
void emsg(const char *fmt, ...);
void equit(const char *fmt, ...);

int event_del(int epfd, int fd);
int event_add(int epfd, int fd, int op);
int event_mod(int epfd, int fd, int op);

int handle_read(client_t *cli);
int handle_write(client_t *cli);

void webchild(int);

#endif /* HASP_H__ */
