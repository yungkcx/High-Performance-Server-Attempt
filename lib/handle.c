#include "../include/hpsa.h"

int handle_read(client_t *cli)
{
    ssize_t n;

    errno = 0;
    if (cli->str.data == NULL)
        cli->str.data = malloc(MAXLINE);
    if ((n = readn(cli->sockfd, cli->str.data, MAXLINE)) <= 0)
        return READ_FAILURE;
    cli->str.len = n;
    return READ_OK;
}

int handle_write(client_t *cli)
{
    ssize_t n;

    if ((n = writen(cli->sockfd, cli->str.data, cli->str.len)) <= 0) {
            return WRITE_FAILURE;
    }
    if (cli->resource == 0) {
        return WRITE_OK;
    } else {
        while ((n = readn(cli->resource, cli->str.data, cli->str.len)) > 0)
            writen(cli->sockfd, cli->str.data, n);
    }
    close(cli->resource);
    cli->str.len  = 0;
    cli->resource = 0;
    return WRITE_OK;
}
