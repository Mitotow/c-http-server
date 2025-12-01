#include "../lib/filesystem.h"
#include "../router.h"
#include <stdbool.h>
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
#define HTTP_HEAD "HEAD"
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

// Content-Type text
#define CTYPE_TEXT_PREFIX "text/"
#define CTYPE_PLAIN "text/plain"
#define CTYPE_HTML "text/html"
#define CTYPE_JS "text/javascript"
#define CTYPE_CSS "text/css"
#define CTYPE_CSV "text/csv"
#define CTYPE_XML "test/xml"
// Content-Type Image
#define CTYPE_ICO "image/x-icon"
#define CTYPE_GIF "image/gif"
#define CTYPE_JPEG "image/jpeg"
#define CTYPE_PNG "image/png"
#define CTYPE_TIFF "image/tiff"
#define CTYPE_SVG "image/svg+xml"
// Content-Type Font
#define CTYPE_WOFF "font/woff"
#define CTYPE_WOFF2 "font/woff2"
#define CTYPE_TTF "application/x-font-ttf"
#define CTYPE_OTF "application/x-font-opentype"
#define CTYPE_EOT "application/vnd.ms-fontobject"

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
  const char *ext;
  const char *type;
} content_type_t;

static const status_t status[] = {
    {OK, "Ok"},
    {CREATED, "Created"},
    {BAD_REQUEST, "Bad Request"},
    {NOT_FOUND, "Not Found"},
    {INTERNAL_SERVER_ERROR, "Internal Server Error"},
};

static const content_type_t content_types[] = {
    {EXT_HTML, CTYPE_HTML},     {EXT_JS, CTYPE_JS},
    {EXT_CSS, CTYPE_CSS},       {EXT_CSV, CTYPE_CSV},
    {EXT_XML, CTYPE_XML},       {EXT_ICO, CTYPE_ICO},
    {EXT_GIF, CTYPE_GIF},       {EXT_JPEG, CTYPE_JPEG},
    {EXT_JPEG_ALT, CTYPE_JPEG}, {EXT_PNG, CTYPE_PNG},
    {EXT_TIFF, CTYPE_TIFF},     {EXT_SVG, CTYPE_SVG},
    {EXT_WOFF, CTYPE_WOFF},     {EXT_WOFF2, CTYPE_WOFF2},
    {EXT_TTF, CTYPE_TTF},       {EXT_OTF, CTYPE_OTF},
    {EXT_EOT, CTYPE_EOT},
};

const status_t *getStatus(int code);
const char *getContentType(char *ext);
bool isTextContentType(char *ctype);

#endif
