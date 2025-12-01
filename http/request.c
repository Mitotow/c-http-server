#include "request.h"
#include "../utils/strutils.h"
#include "http.h"
#include <string.h>

char **getHeaderField(request_t *req, char *key) {
  if (strcmp(key, "Host") == 0) {
    return &req->host;
  } else if (strcmp(key, "User-Agent") == 0) {
    return &req->userAgent;
  } else if (strcmp(key, "Accept") == 0) {
    return &req->accept;
  } else if (strcmp(key, "Connection") == 0) {
    return &req->connection;
  } else if (strcmp(key, "Content-Type") == 0) {
    return &req->contentType;
  }

  return NULL;
}

void safeSet(char *value, char **dest) {
  if (dest != NULL && value != NULL) {
    *dest = strdup(trimOnlySpaceAndTab(value));
  }
}

// Check if every required parameters are given in a request
bool isValidRequest(request_t *req) {
  if (req == NULL)
    return false;

  // Check the presence of default HTTP Headers
  if (req->method == NULL || req->route == NULL || req->httpVersion == NULL) {
    return false;
  }

  // Check if the host argument is present
  if (req->host == NULL) {
    return false;
  }

  // Check if the content is well initialized (with contentType and
  // contentLength)
  if (req->content != NULL && req->contentLength == 0L &&
      req->contentType == NULL) {
    return false;
  }

  return true;
}

// Normalize the request path to avoid security problems
void normalizeRequestRoute(request_t *req) {
  if (req->route == NULL || req->route[0] == '\0') {
    free(req->route);
    req->route = strdup("/");
    return;
  }
  if (req->route[0] != '/') {
    char *slash = strchr(req->route, '/');
    if (slash != NULL) {
      free(req->route);
      req->route = strdup(slash);
    }
  }
}

void readHeader(request_t *req, char *line) {
  int index = 0;
  char *token;
  char *tokens[3] = {NULL, NULL, NULL};

  while (index < 3 && (token = strtok_r(line, " ", &line))) {
    tokens[index] = token;
    index++;
  }

  safeSet(tokens[0], &req->method);
  safeSet(tokens[1], &req->route);
  safeSet(tokens[2], &req->httpVersion);
}

void readHeaders(request_t *req, char *line) {
  int index = 0;
  char *token;
  char *tokens[2] = {NULL, NULL};

  while (index < 2 && (token = strtok_r(line, ":", &line))) {
    tokens[index] = token;
    index++;
  }

  char **dest = getHeaderField(req, tokens[0]);
  safeSet(tokens[1], dest);
}

void printDebugRequest(request_t *req) {
  printf("%s %s %s\n", req->method, req->route, req->httpVersion);
  printf("User-Agent: %s\n", req->userAgent);
  printf("Connection: %s\n", req->connection);
  printf("Accept: %s\n", req->accept);
  printf("Accept-Language: %s\n", req->acceptLanguage);
  printf("Accept-Encoding: %s\n", req->acceptEncoding);
}

// Read HTTP request
void readRequest(char *buffer, request_t *req) {
  bool isHeader = true;
  char *rest = malloc(strlen(buffer) + 1);
  char *token;

  strcpy(rest, buffer);
  token = strtok(rest, HTTP_DELIMITER);
  while (token != NULL) {
    if (strcmp(token, "") == 0 && isHeader) {
      isHeader = false;
    } else if (isHeader) {
      readHeader(req, token);
      isHeader = false;
    } else {
      readHeaders(req, token);
    }

    token = strtok(NULL, HTTP_DELIMITER);
  }

  normalizeRequestRoute(req);

  free(rest);
}

request_t *createRequest(char *buffer) {
  request_t *req = (request_t *)calloc(1, sizeof(request_t));
  readRequest(buffer, req);
  return req;
}

// Free all of the elements inside the request and the request itself
void destroyRequest(request_t *req) {
  if (req == NULL) {
    return;
  }

  free(req);
}
