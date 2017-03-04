#include "hpsa.h"

int main()
{
    int listenfd, status;
    pid_t pid[NCPU];
    hpsa_config conf;

    puts("Reading hpsa.conf...");
    if (config_read(&conf) < 0)
        exit(1);
    if ((listenfd = tcp_listen(conf.host, conf.port, NULL, conf.ncpu * CONN_PER_PROCESS)) < 0)
        esys("tcp_listen error");
    for (int i = 0; i < conf.ncpu; ++i) {
        if ((pid[i] = fork()) == 0) {
            webchild(listenfd);
            printf("%d\n", getpid());
            exit(0);
        } else if (pid[i] < 0) {
            for (int j = 0; j < i; ++j)
                kill(pid[j], SIGKILL);
            esys("fork error");
        }
    }
    signal(SIGPIPE, SIG_IGN);
    printf("----- Start At %s:%s -----\n", conf.host, conf.port);
    for (int i = 0; i < NCPU; ++i) {
        wait(&status);
        if (WIFEXITED(status)) {
            printf("child %d exit: %d\n", pid[i], WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("child %d terminated by signal %d\n", pid[i], WTERMSIG(status));
        }
    }

    return 0;
}
