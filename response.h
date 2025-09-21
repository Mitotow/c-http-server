#ifndef RESPONSE_H
#define RESPONSE_H

#include "http.h"
#include "request.h"

#define RESPONSE_HEADER_SIZE 512

typedef struct {
  char *httpVersion;
  status_t status;
  long contentLength;
  char *contentType;
  char *connection;
  char *content;
} response_t;

ssize_t sendResponse(int client_socket, response_t res);
void addDefaultHeadersResponse(request_t req, response_t *res);
void createResponse(request_t req, response_t *res, int statusCode);
void createContentResponse(request_t req, response_t *res, char *cnotentType,
                           char *content, long length);

#endif
