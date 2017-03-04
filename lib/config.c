#include "../hpsa.h"

static const char* config_whitespace(const char* p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        ++p;
    return p;
}

static int config_complete(hpsa_config* conf, const char* buf, const char* pos)
{
    pos = config_whitespace(pos);
    if (STRCMP(buf, ==, "HOST"))
        strncpy(conf->host, pos, 32);
    else if (STRCMP(buf, ==, "PORT"))
        strncpy(conf->port, pos, 8);
    else if (STRCMP(buf, ==, "NCPU"))
        conf->ncpu = atoi(pos);
    else 
        return -1;
    return 0;
}

int config_read(hpsa_config* conf)
{
    FILE* f;
    char* pos;
    char  buf[CONFIG_LINE];
    int   line;
    int   ret;

    f = fopen("hpsa.conf", "r");
    if (f == NULL)
        esys("Open hpsa.conf error");

    line = ret = 0;
    while (fgets(buf, CONFIG_LINE, f) != NULL) {
        line++;

        if (buf[0] == '#')
            continue;
        pos = buf;
        while (*pos != '\0') {
            if (*pos == '\n') {
                *pos = '\0';
                break;
            }
            pos++;
        }
        if (buf[0] == '\0')
            continue;

        pos = strchr(buf, '=');
        if (pos == NULL) {
            emsg("Syntax error at line: %d", line);
            return -1;
        }
        *pos = '\0';
        pos++;
        ret = config_complete(conf, buf, pos);
        if (ret < 0) {
            emsg("Cannot parse `%s' at line: %d", buf, line);
            return -1;
        }
    }
    return 0;
}
