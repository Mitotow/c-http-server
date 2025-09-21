#include <stdbool.h>
#ifndef REQUEST_H
#define REQUEST_H

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

struct request {
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
};

bool isValidRequest(request_t req);
void readRequest(char *buffer, request_t *req);

#endif
