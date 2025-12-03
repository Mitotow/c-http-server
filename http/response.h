#include "http.h"
#include "request.h"

#ifndef RESPONSE_H
#define RESPONSE_H
#define RESPONSE_HEADER_SIZE 512
#define RESPONSE_DATE_SIZE 64

typedef struct {
  char *httpVersion;
  status_t status;
  long contentLength;
  char *date;
  char *contentType;
  char *connection;
  char *content;
} response_t;

ssize_t sendResponse(int client_socket, response_t *res);
void addDefaultHeadersResponse(request_t *req, response_t *res);
response_t *createResponse(request_t *req, int statusCode);
response_t *createContentResponse(request_t *req, char *cnotentType,
                                  char *content, long length);
void destroyResponse(response_t *res);

#endif
