#ifndef HTTP_PARSE_H__
#   define HTTP_PARSE_H__

#define MAXHEADER 256

enum {
    GET,
    POST, /* Actually, we don't support POST AND HEAD */
    HEAD
};

enum {
    HTTP_PARSE_OK,
    HTTP_PARSE_BAD_REQ,
    HTTP_PARSE_BAD_SYNTAX,
    MAKE_REPLY_OK,
    MAKE_REPLY_FAILURE,
};

typedef struct {
    int method;
    hps_str_t resource;
    int version; /* 1 for 1.1, 0 for 1.0, 2 for 2.0 */
} http_req;

typedef struct {
    /* Yes! Nothing! */
} http_headers;

int http_parse(client_t *cli);

#endif /* HTTP_PARSE_H__ */
