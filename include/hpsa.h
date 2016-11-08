#ifndef HASP_H__
#	define HASP_H__

#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_link.h>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>

#include <sys/times.h>
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <net/if.h>
#include <net/ethernet.h>

#include <arpa/inet.h>

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <setjmp.h>
#include <err.h>
#include <inttypes.h>
#include <sys/utsname.h>
#include <wchar.h>
#include <aio.h>
#include <dirent.h>
#include <ifaddrs.h>
#include <syslog.h>
#include <pthread.h>
#include <ctype.h>
#include <error.h>
#include <math.h>
#include <grp.h>
#include <pwd.h>
#include <shadow.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <termio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>

#include "clients.h"
#include "http_parse.h"

#define VERSION           "1.0"
#define NCPU              8
#define MAXLINE           4096
#define TIMEBUF           40
#define PORT              "33333"
#define CONN_PER_PROCESS  200
#define LISTENQ           (CONN_PER_PROCESS) * (NCPU)

#define UNUSED(v)            (void)(v);
#define max(a, b) ({\
		typeof(a) _max1 = (a);\
		typeof(b) _max2 = (b);\
		_max1 > _max2 ? _max1 : _max2;})
#define STRCMP(a, R, b)          (strcmp(a, b) R 0)
#define STRNCMP(a, R, b, n)      (strncmp(a, b, n) R 0);
#define STRCASECMP(a, R, b)      (strcasecmp(a, b) R 0)
#define STRNCASECMP(a, R, b, n)  (strncasecmp(a, b, n) R 0)
#define debug(M, ...)        fprintf(stderr, "DEBUG %s (in function '%s'):%d: " M "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

enum {
    WRITE_OK,
    WRITE_AGAIN,
    WRITE_FAILURE,
    READ_OK,
    READ_AGAIN,
    READ_FAILURE
};

const char *sock_ntop(const struct sockaddr *sa);
int tcp_connect(const char *host, const char *port);
int tcp_listen(const char *host, const char *port, socklen_t *addrlenp);
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

#endif /* HASP_H__ */
