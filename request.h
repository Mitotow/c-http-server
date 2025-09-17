#include <stdbool.h>
#ifndef REQUEST_H
#define REQUEST_H

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

bool isValidRequest(struct request req);
void readRequest(char *buffer, struct request *req);

#endif
