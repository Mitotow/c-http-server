#include "router.h"
#include <sys/socket.h>

#ifndef HTTP_H
#define HTTP_H

// HTTP Parameters
#define HTTP_DELIMITER "\r\n"
#define HTTP_REQ_BUFFER_SIZE 4096
#define HTTP_MAX_HEADER_SIZE 512
#define HTTP_DEFAULT_VERSION "HTTP/1.1"
#define HTTP_1_0 "HTTP/1.0"

// HTTP Methods
#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_PUT "PUT"
#define HTTP_DELETE "DELETE"

// Status Code
#define OK 200
#define CREATED 201
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define INTERNAL_SERVER_ERROR 500

#define DEFAULT_STATUS_TEXT ""

// Content-Type
#define CTYPE_PLAIN "text/plain"
#define CTYPE_HTML "text/html"
#define CTYPE_JS "text/javascript"
#define CTYPE_CSS "text/css"
#define CTYPE_CSV "text/csv"
#define CTYPE_XML "test/xml"
#define CTYPE_ICO "image/x-icon"
#define CTYPE_GIF "image/gif"
#define CTYPE_JPEG "image/jpeg"
#define CTYPE_PNG "image/png"
#define CTYPE_TIFF "image/tiff"

// Connection
#define CONN_CLOSE "close"
#define CONN_KEEP_ALIVE "keep-alive"

// Keep-Alive
#define KEEP_ALIVE_TIMEOUT 5 // 5 seconds before timeout
#define KEEP_ALIVE_MAX 100   // Max requests per session

typedef struct {
  int code;
  const char *text;
} status_t;

typedef struct {
  int server_fd;
  char *static_dir;
  struct sockaddr *address;
  socklen_t *addrlen;
  router_t *router;
} server_context_t;

static const status_t status[] = {
    {OK, "Ok"},
    {CREATED, "Created"},
    {BAD_REQUEST, "Bad Request"},
    {NOT_FOUND, "Not Found"},
    {INTERNAL_SERVER_ERROR, "Internal Server Error"},
};

const status_t *getStatus(int code);
char *getContentType(char *ext);

#endif
