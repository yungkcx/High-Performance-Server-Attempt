#ifndef CONFIG_H__
# define CONFIG_H__

#define CONFIG_LINE       256

#define NCPU              8
#define HOST              "localhost"
#define PORT              "8080"

typedef struct {
    char host[32];
    char port[8];
    int  ncpu;
} hpsa_config;

int config_read(hpsa_config* conf);

#endif /* CONFIG_H__ */
