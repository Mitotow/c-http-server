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

// Content-Type
#define CTYPE_HTML "text/html"
#define CTYPE_ICO "image/x-icon"

// Connection
#define CONN_CLOSE "close"
#define CONN_KEEP_ALIVE "keep-alive"

// Keep-Alive
#define KEEP_ALIVE_TIMEOUT 5 // 5 seconds before timeout
#define KEEP_ALIVE_MAX 100   // Max requests per session

#endif
