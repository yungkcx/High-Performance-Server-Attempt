#include "../include/hpsa.h"

int handle_read(client_t *cli)
{
    ssize_t n;

    errno = 0;
    if (cli->str.data == NULL)
        cli->str.data = malloc(MAXLINE);
    if ((n = readn(cli->sockfd, cli->str.data, MAXLINE)) <= 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return READ_OK;
        return READ_FAILURE;
    }
    cli->str.len = n;
    return READ_OK;
}

int handle_write(client_t *cli)
{
    ssize_t n;

    if ((n = writen(cli->sockfd, cli->str.data, cli->str.len)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return WRITE_AGAIN;
        else
            return WRITE_FAILURE;
    }
    cli->str.len = 0;
    return WRITE_OK;
}
