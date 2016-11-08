#ifndef CLIENTS_H__
# define CLIENTS_H__

typedef struct {
    size_t len;
    char *data;
} hps_str_t;

typedef struct client_t client_t;
struct client_t {
    int sockfd;
    struct timespec time;
    hps_str_t str;
    client_t *next;
};

/* Use a hash table to save clients' infomation */
typedef struct clients_list clients_list;
struct clients_list {
    int now;
    int size;
    client_t **hashtbl;
};

/* 5000 ms for timeout */
#ifndef TIMEOUT
#   define TIMEOUT 1000L
#endif

#define hps_str_null(str)\
    do {\
        (str)->len = 0;\
        (str)->data = NULL;\
    } while (0)

client_t *clients_find(clients_list *t, int sockfd);
int clients_init(clients_list *t);
int clients_add(clients_list *t, int sockfd);
void clients_del(int epfd, clients_list *t, int sockfd);
void clients_timeout(clients_list *t, int epfd);
void clients_update_timeout(clients_list *t, int sockfd);

#endif /* CLIENTS_H__ */
