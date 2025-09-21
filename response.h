#include "request.h"
#include <stdlib.h>

#ifndef RESPONSE_H
#define RESPONSE_H

#define RESPONSE_HEADER_SIZE 512

#define OK 200
#define CREATED 201
#define BAD_REQUEST 400
#define NOT_FOUND 404
#define INTERNAL_SERVER_ERROR 500

typedef struct {
  char *httpVersion;
  int statusCode;
  char *statusName;
  long contentLength;
  char *contentType;
  char *connection;
  char *content;
} response_t;

struct response {
  char *httpVersion;
  int statusCode;
  char *statusName;
  long contentLength;
  char *contentType;
  char *connection;
  char *content;
};

ssize_t sendResponse(int client_socket, response_t res);
void addDefaultHeadersResponse(request_t req, response_t *res);
void createResponse(request_t req, response_t *res, int statusCode);
void createContentResponse(request_t req, response_t *res, char *cnotentType,
                           char *content, long length);

#endif
