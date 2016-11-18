#include "../include/hpsa.h"

int set_fl(int fd, int flags)
{
    /* Cant't be used to set O_SYNC or O_DSYNC. */
    int val;
    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        return val;
    val |= flags;
    return fcntl(fd, F_SETFL, val);
}

int clr_fl(int fd, int flags)
{
    int val;
    if ((val = fcntl(fd, F_GETFL, 0)) < 0)
        return val;
    val &= ~flags;
    return fcntl(fd, F_SETFL, val);
}


const char *sock_ntop(const struct sockaddr *sa)
{
	static char str[45]; /* X:X:X:X:X:X:a.b.c.d, so it is 45 characters */
	char portstr[8];

	switch (sa->sa_family) {
		case AF_INET: {
			struct sockaddr_in *sin = (struct sockaddr_in *) sa;
			if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
				return NULL;
			snprintf(portstr, sizeof(portstr), ":%hu", ntohs(sin->sin_port));
			strcat(str, portstr);
			return str;
		}
		case AF_INET6: {
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
            str[0] = '[';
			if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL)
				return NULL;
			snprintf(portstr, sizeof(portstr), "]:%hu", ntohs(sin6->sin6_port));
			strcat(str, portstr);
			return str;
		}
		default:
			snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d", sa->sa_family);
			return str;
	}
	return NULL;
}

/* handle SIGINTR for accept */
int accept_e(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int connfd;

	while (1) {
		if ((connfd = accept(sockfd, addr, addrlen)) < 0) {
			if (errno == EINTR) {
				continue;
            } else {
				eret("accept error");
                return -1;
            }
		} else {
            if (set_fl(connfd, O_NONBLOCK) < 0) {
                eret("set_fl error");
                return -1;
            }
			return connfd;
		}
	}
}

ssize_t writen(int fd, const void *buf, size_t n)
{
	ssize_t nwrite;
    size_t  nleft;

    nleft = n;
	while (nleft > 0) {
		if ((nwrite = write(fd, buf, nleft)) < 0) {
			if (errno == EINTR) {
				nwrite = 0;
            } else if (errno == EPIPE) {
                return nwrite;
            } else {
                eret("writen error");
                return nwrite;
			}
		} else if (nwrite == 0) {
			break;
		}
        nleft -= nwrite;
        buf   += nwrite;
	}
	return n - nleft;
}

ssize_t readn(int fd, void *buf, size_t n)
{
	ssize_t nread;
    size_t  nleft;

    nleft = n;
    while (nleft > 0) {
		if ((nread = read(fd, buf, nleft)) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else if (errno == EINTR) {
				nread = 0;
            } else {
                debug("readn error");
                return nread;
			}
		} else if (nread == 0) {
			break;
		}
        nleft -= nread;
        buf   += nread;
	}
	return n - nleft;
}

int tcp_connect(const char *host, const char *port)
{
	int fd, n;
	struct addrinfo hints, *res, *ressave;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((n = getaddrinfo(host, port, &hints, &res)) != 0) {
		eret("tcp_connect error for %s, %s: %s", host, port, gai_strerror(n));
        return -1;
    }
	ressave = res;

	do {
		fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (fd < 0)
			continue;
		if (connect(fd, res->ai_addr, res->ai_addrlen) == 0)
			break;         /* Success */
		if (close(fd) < 0)
			esys("close error");
	} while ((res = res->ai_next) != NULL);

	if (res == NULL) {
		eret("tcp_connect error for %s, %s", host, port);
        return -1;
    }

	freeaddrinfo(ressave);

	return fd;
}

/* addrlenp can be NULL, it is not necessary for TCP */
int tcp_listen(const char *host,
		const char *port, socklen_t *addrlenp)
{
	int fd, n;
	struct addrinfo hints, *res, *ressave;
	const int on = 1;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((n = getaddrinfo(host, port, &hints, &res)) != 0) {
		eret("tcp_listen error for %s, %s: %s", host, port, gai_strerror(n));
        return -1;
    }
	ressave = res;

	do {
		fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (fd < 0)
			continue;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0) {
			eret("setsockopt error");
            return -1;
        }
		if (bind(fd, res->ai_addr, res->ai_addrlen) == 0)
			break;
		if (close(fd) < 0) {
			eret("close error");
            return -1;
        }
	} while ((res = res->ai_next) != NULL);

	if (res == NULL) {
		eret("tcp_listen error for %s, %s", host, port);
        return -1;
    }
	if (listen(fd, LISTENQ) < 0) {
		eret("listen error");
        return -1;
    }
	if (addrlenp)
		*addrlenp = res->ai_addrlen;

	freeaddrinfo(ressave);

	return fd;
}

/* error handler functions */
static void edoit(int status, const char *fmt, va_list ap)
{
    int errno_save, n;
    char buf[MAXLINE + 1];

    errno_save = errno;
    vsnprintf(buf, MAXLINE, fmt, ap);
    n = strlen(buf);
    if (status)
        snprintf(buf + n, MAXLINE - n, ": %s", strerror(errno_save));
    strcat(buf, "\n");
    fputs(buf, stderr);
    fflush(stderr);
}

void equit(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    edoit(0, fmt, ap);
    va_end(ap);
    exit(EXIT_SUCCESS);
}

void esys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    edoit(1, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

void eret(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    edoit(1, fmt, ap);
    va_end(ap);
}

void emsg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    edoit(0, fmt, ap);
    va_end(ap);
}
