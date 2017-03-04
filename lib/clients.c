#include "../hpsa.h"

/* Hash table functions. */
static int hash(int n, int tablesize)
{
    return (n % tablesize);
}

static int next_prime(int n)
{
    int sign;

    for ( ; ; ++n) {
        sign = 1;
        for (int i = 2; i < n / 2; ++i) {
            if (n % i == 0) {
                sign = 0;
                break;
            }
        }
        if (sign == 1)
            return n;
    }
}

/* APIs */
int clients_init(clients_list *t)
{
    t->now = 0;
    t->size = next_prime(CONN_PER_PROCESS);
    t->hashtbl = malloc(sizeof(client_t*) * t->size);
    for (int i = 0; i < t->size; ++i)
        t->hashtbl[i] = NULL;
    return 0;
}

#define SET_CLIENT(c, sockfd)\
    do {\
        c->sockfd = sockfd;\
        struct timespec tp;\
        clock_gettime(CLOCK_REALTIME, &tp);\
        c->time.tv_sec = tp.tv_sec + TIMEOUT / 1000;\
        c->time.tv_nsec = tp.tv_nsec + (TIMEOUT % 1000) * 1000000L;\
        hps_str_null(&c->str);\
        c->next = NULL;\
    } while (0)

int clients_add(clients_list *t, int sockfd)
{
    int pos;
    client_t *c, *tmp, *node;

    if (t->now >= CONN_PER_PROCESS) {
        return -1;
    }
    pos = hash(sockfd, t->size);
    c = t->hashtbl[pos];
    while (c != NULL && c->sockfd != sockfd) {
        tmp = c;
        c = c->next;
    }
    if (c == NULL) {
        node = malloc(sizeof(client_t));
        SET_CLIENT(node, sockfd);
        if (t->hashtbl[pos] == NULL)
            t->hashtbl[pos] = node;
        else
            tmp->next = node;
        ++t->now;
    } else { /* will never execute */
        fprintf(stderr, "NOOOOOOOO!!!\n");
        return -1;
    }
    return 0;
}

static void client_free(client_t *c)
{
    if (c == NULL)
        return;
    if (c->str.data != NULL)
        free(c->str.data);
    free(c);
}

void clients_del(int epfd, clients_list *t, int sockfd)
{
    int pos;
    client_t *c, *tmp;

    pos = hash(sockfd, t->size);
    tmp = c = t->hashtbl[pos];
    while (c != NULL && c->sockfd != sockfd) {
        tmp = c;
        c = c->next;
    }
    if (c != NULL) {
        if (tmp == c)
            t->hashtbl[pos] = c->next;
        else
            tmp->next = c->next;
        event_del(epfd, sockfd);
        close(c->sockfd);
        client_free(c);
        --t->now;
    }
}

void clients_timeout(clients_list *t, int epfd)
{
    client_t *c, *tmp;
    struct timespec tp;

    clock_gettime(CLOCK_REALTIME, &tp);
    for (int i = 0; i < t->size; ++i) {
        c = t->hashtbl[i];
        while (c != NULL) {
            if (c->time.tv_sec <= tp.tv_sec && c->time.tv_nsec <= tp.tv_nsec) {
                tmp = c->next;
                clients_del(epfd, t, c->sockfd);
                c = tmp;
            } else {
                c = c->next;
            }
        }
    }
}

client_t *clients_find(clients_list *t, int sockfd)
{
    client_t *c;

    c = t->hashtbl[hash(sockfd, t->size)];
    while (c != NULL && c->sockfd != sockfd)
        c = c->next;
    return c;
}

void clients_update_timeout(clients_list *t, int sockfd)
{
    client_t *c;
    struct timespec tp;

    clock_gettime(CLOCK_REALTIME, &tp);
    if ((c = clients_find(t, sockfd)) != NULL) {
        c->time.tv_sec  = tp.tv_sec + (TIMEOUT / 1000);
        c->time.tv_nsec = tp.tv_nsec + (TIMEOUT % 1000) * 1000000L;
    }
}
