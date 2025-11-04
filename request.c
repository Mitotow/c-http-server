#include "request.h"
#include "http.h"
#include <string.h>

// Fetch the HTTP request header to setup the request struct
void fetchRequestHeader(char *token, request_t *req) {
  int index = 0;
  char *rest = token;
  char *tokens[3] = {NULL, NULL, NULL};

  while ((token = strtok_r(rest, " ", &rest))) {
    tokens[index] = token;
    index++;
  }

  if (tokens[0] != NULL)
    req->method = strdup(tokens[0]);
  if (tokens[1] != NULL)
    req->route = strdup(tokens[1]);
  if (tokens[2] != NULL)
    req->httpVersion = strdup(tokens[2]);

  if (req->connection == NULL) {
    if (strcmp(req->httpVersion, HTTP_1_0)) {
      req->connection = strdup(CONN_CLOSE);
    } else {
      req->connection = strdup(CONN_KEEP_ALIVE);
    }
  }
}

// Fetch the HTTP request content to setupt the request struct
void fetchRequestContent(char *token, request_t *req) {
  char *rest = token;
  char *key = strtok_r(rest, ": ", &rest);
  if (key == NULL)
    return;

  char *value = strtok_r(NULL, "", &rest);
  if (value == NULL)
    return;

  if (strcmp(key, "Host") == 0) {
    req->host = strdup(value);
  } else if (strcmp(key, "User-Agent") == 0) {
    req->userAgent = strdup(value);
  } else if (strcmp(key, "Accept") == 0) {
    req->accept = strdup(value);
  } else if (strcmp(key, "Connection") == 0) {
    req->connection = strdup(value);
  } else if (strcmp(key, "Content-Type") == 0) {
    req->contentType = strdup(value);
  }
}

// Read a line of the HTTP request and dispatch it between 2 functions
void readRequestLine(char *token, request_t *req, bool isHeader) {
  if (isHeader == true) {
    fetchRequestHeader(token, req);
  } else {
    fetchRequestContent(token, req);
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

// Read HTTP request
void readRequest(char *buffer, request_t *req) {
  bool isHeader = true;
  char *rest = strdup(buffer);
  char *rest_ptr = rest;
  char *token;
  while ((token = strtok_r(rest, HTTP_DELIMITER, &rest))) {
    char *cp = strdup(token);
    if (strlen(token) == 0) {
      strncpy(req->content, rest, sizeof(req->content) - 1);
      req->content[sizeof(req->content) - 1] = '\0';
      break;
    }
    readRequestLine(cp, req, isHeader);
    if (isHeader)
      isHeader = false;
    free(cp);
  }
  free(rest_ptr);

  normalizeRequestRoute(req);
}

// Free all of the elements inside the request and the request itself
void destroyRequest(request_t *req) {
  if (req == NULL) {
    return;
  }

  if (req->method != NULL) {
    free(req->method);
  }
  if (req->route != NULL) {
    free(req->route);
  }
  if (req->httpVersion != NULL) {
    free(req->httpVersion);
  }
  if (req->host != NULL) {
    free(req->host);
  }
  if (req->userAgent != NULL) {
    free(req->userAgent);
  }
  if (req->accept != NULL) {
    free(req->accept);
  }
  if (req->connection != NULL) {
    free(req->connection);
  }
  if (req->contentType != NULL) {
    free(req->contentType);
  }

  free(req);
}
