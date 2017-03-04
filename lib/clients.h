#ifndef CLIENTS_H__
# define CLIENTS_H__

typedef struct {
    size_t len;
    char *data;
} hps_str_t;

typedef struct client_t client_t;
struct client_t {
    int             sockfd;
    struct timespec time;   /* For timeout event. */
    hps_str_t       str;    /* Received or processed HTTP headers. */
    int             resource; /* File descriptor of the resource will be sended. */
    client_t        *next;
};

/* Use a hash table to save clients' infomation */
typedef struct clients_list clients_list;
struct clients_list {
    int        now;   /* The number of connections. */
    int        size;  /* The maximum number of connections. */
    client_t **hashtbl;
};

/* 2000 ms for timeout */
#ifndef TIMEOUT
#   define TIMEOUT 2000L
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
