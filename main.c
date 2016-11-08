#include "./include/hpsa.h"

void webchild(int);
int main()
{
    int listenfd, status;
    pid_t pid[NCPU];

    if ((listenfd = tcp_listen("localhost", PORT, NULL)) < 0)
        esys("tcp_listen error");
    for (int i = 0; i < NCPU; ++i) {
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
    puts("----- Start OK -----");
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
