#include "../include/hpsa.h"

static int parse_req(hps_str_t *str, http_req *req)
{
    char *p;

    if (STRNCASECMP(str->data, ==, "GET", 3)) {
        req->method = GET;
        p = str->data + 3;
    } else if (STRNCASECMP(str->data, ==, "POST", 4)) {
        req->method = POST;
        p = str->data + 4;
    } else if (STRNCASECMP(str->data, ==, "HEAD", 4)) {
        req->method = HEAD;
        p = str->data + 4;
    } else {
        return HTTP_PARSE_BAD_SYNTAX;
    }
    if (*p == ' ')
        ++p;
    else
        return HTTP_PARSE_BAD_SYNTAX;
    req->resource.data = p;
    req->resource.len = 0;
    if (*p != '/')
        return HTTP_PARSE_BAD_SYNTAX;
    for ( ; *p != ' ' && *p != 0 && *p != '\r' && *p != '\n'; ++p)
        ++req->resource.len;
    if (*p++ != ' ')
        return HTTP_PARSE_BAD_SYNTAX;
    if (STRNCASECMP(p, ==, "HTTP/1.1", 8))
        req->version = 1;
    else if (STRNCASECMP(p, ==, "HTTP/1.0", 8))
        req->version = 0;
    else if (STRNCASECMP(p, ==, "HTTP/2.0", 8))
        req->version = 2;
    else
        return HTTP_PARSE_BAD_SYNTAX;
    return HTTP_PARSE_OK;
}

static char *parse_get_line(char *src, char *buf)
{
    char *save;

    save = src;
    for ( ; (*buf = *src) != '\n' && *buf != 0; ++src, ++buf)
        ;
    if (src - save == 1) { /* blank line, headers end */
        return NULL;
    } else {
        ++src;
        return src;
    }
}

static int parse_headers(hps_str_t *str, http_headers *headers)
{
    char buf[MAXHEADER];
    char *p;

    p = parse_get_line(str->data, buf); /* skip request line */
    while ((p = parse_get_line(p, buf)) != NULL) {
        /* The server is a so simple that don't need to parse headers. */
    }
    return HTTP_PARSE_OK;
}

/* Convert local time to RFC1123 format. */
static void rfctime(char *buf, const struct tm *tm)
{
	static char *days[] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static char *months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};
	snprintf(buf, TIMEBUF, "%s, %02d %s %04d %02d:%02d:%02d GMT",
			days[tm->tm_wday], tm->tm_mday, months[tm->tm_mon - 1],
			tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

static off_t fsize(int dirfd, const char *path)
{
    struct stat st;
    if (fstatat(dirfd, path, &st, 0) < 0)
        return -1;
    return st.st_size;
}

static void getsuffix(const char *path, char *suffix)
{
	size_t size;

	size = strlen(path);
	for (; size > 0; --size) {
		if (path[size - 1] == '.') {
			strcpy(suffix, path + size - 1);
			return;
		}
	}
	strcpy(suffix, "*");
}

enum {
    ISREGULAR,
    ISPHP
};

static int gettype(const char *path, char **type)
{
	char suffix[20];
	static char *typelist[][2] = {
		{"*"    , "application/octet-stream"},
        {".php" , "text/html"},
		{".html", "text/html"},
		{".htm" , "text/html"},
		{".htx" , "text/html"},
		{".avi" , "video/avi"},
		{".bmp" , "application/x-bmp"},
		{".doc" , "application/msword"},
		{".dot" , "application/msword"},
		{".exe" , "application/x-msdownload"},
		{".fo"  , "text/xml"},
		{".ico" , "image/x-icon"},
		{".img" , "application/x-img"},
		{".java", "java/*"},
		{".jpg" , "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".jsp" , "text/html"},
		{".mp2" , "audio/mp2"},
		{".mp3" , "auido/mp3"},
		{".mp4" , "video/mpeg4"},
		{".mpeg", "video/mpg"},
		{".pdf" , "application/pdf"},
		{".png" , "image/png"},
		{".xhtml", "text/html"},
		{".css" , "text/css"},
		{".xml" , "text/xml"},
		{".txt" , "text/plain"},
		{".js"  , "text/javascript"},
		{"."    , "x-"},
		{NULL   , NULL},
	};

	getsuffix(path, suffix);
	for (int i = 0; typelist[i][0] != NULL; ++i) {
		if (STRCMP(typelist[i][0], ==, suffix)) {
			*type = typelist[i][1];
            if (STRNCMP(typelist[i][0], ==, ".php", 4))
                return ISPHP;
            else
                goto regular_files;
        }
    }
    *type = typelist[0][1];
regular_files:
    return ISREGULAR;
}

static int make_reply(client_t *cli, http_req *req, http_headers *headers)
{
    char   buf[MAXLINE];
    char   timebuf[TIMEBUF];
    char   *path;
    off_t  content_len;
    char   *content_type;
    int    code;
    time_t t;
    int    fd, dirfd;
    hps_str_t *str;

    str = &cli->str;
    code = 200;
    req->resource.data[req->resource.len] = '\0';
    if (STRCMP(req->resource.data, ==, "/"))
        path = "index.html";
    else
        path = req->resource.data + 1;
    if ((dirfd = open("www", O_RDONLY)) < 0) {
        if (errno == EACCES) {
            fprintf(stderr, "No \"www\" directory!!\n");
            exit(errno);
        }
    }
    {
        struct stat buf;
        if (fstatat(dirfd, path, &buf, 0) < 0 && errno == ENOENT) {
            code = 404;
            path = "status/404.html";
        }
    }
    if (gettype(path, &content_type) == ISPHP) {
        /* Make a temp file for PHP. */
        char s[11] = "www/XXXXXX";
        char tmp_path[255];
        sprintf(tmp_path, "www/%s", path);
        fd = mkstemp(s);
        if (vfork() == 0) {
            close(STDOUT_FILENO);
            dup2(fd, STDOUT_FILENO);
            execlp("php", "php", tmp_path, NULL);
        }
        fsync(fd);
        lseek(fd, 0, SEEK_SET);
        content_len = fsize(dirfd, s + 4);
        unlink(s);
    } else {
        if ((fd = openat(dirfd, path, O_RDONLY)) < 0) {
            eret("Can't open file %s", path);
            return MAKE_REPLY_FAILURE;
        }
        content_len = fsize(dirfd, path);
    }
    t = time(NULL);
    rfctime(timebuf, localtime(&t));
    sprintf(buf, "%s %d %s\r\n"
            "Date: %s\r\n"
            "Server: HighPerformanceServer/%s\r\n"
            "Content-length: %lu\r\n"
            "Content-Type: %s\r\n\r\n",
            req->version == 1 ? "HTTP/1.1" :
            req->version == 0 ? "HTTP/1.0" : "HTTP/2.0",
            code,
            code == 200 ? "OK" : "Not Found", /* Only 200 and 404. */
            timebuf, VERSION,
            content_len, content_type);

    str->len = strlen(buf);
    memcpy(str->data, buf, str->len);
    cli->resource = fd;
    close(dirfd);
    return MAKE_REPLY_OK;
}

int http_parse(client_t *cli)
{
    int ret;
    http_req req;
    http_headers headers;

    if ((ret = parse_req(&cli->str, &req)) != HTTP_PARSE_OK)
        return ret;
    if ((ret = parse_headers(&cli->str, &headers)) != HTTP_PARSE_OK) {
        return ret;
    }
    if ((ret = make_reply(cli, &req, &headers)) != MAKE_REPLY_OK) {
        return ret;
    }

    return HTTP_PARSE_OK;
}
