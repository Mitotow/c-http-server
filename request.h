#ifndef REQUEST_H
#define REQUEST_H

#include <stdbool.h>

typedef struct {
  char *method;
  char *httpVersion;
  char *userAgent;
  char *content;
  char *contentType;
  int contentLength;
  char *route;
  char *host;
  char *connection;
  char *accept;
  char *acceptLanguage;
  char *acceptEncoding;
} request_t;

bool isValidRequest(request_t req);
void readRequest(char *buffer, request_t *req);

#endif
