#include "./include/hpsa.h"

void test_connect(int n)
{
    int i, connfd;
    char buf[MAXLINE];

    for (i = 0; i < n; ++i) {
        connfd = tcp_connect("localhost", PORT);
        write(connfd, "hello", 5);
        read(connfd, buf, MAXLINE);
        puts(buf);
        close(connfd);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
        equit("usage: test <n>");
    time_t t = time(NULL);
    test_connect(atoi(argv[1]));
    printf("Used: %lu s\n", time(NULL) - t);
    return 0;
}
