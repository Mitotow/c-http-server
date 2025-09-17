#include "request.h"
#include "http.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>

// Fetch the HTTP request header to setup the request struct
void fetchRequestHeader(char *token, struct request *req) {
  int index = 0;
  char *rest = token;

  while ((token = strtok_r(rest, " ", &rest))) {
    switch (index) {
    case 0:
      req->method = token;
      break;
    case 1:
      req->route = token;
      break;
    case 2:
      req->httpVersion = token;
      break;
    }

    index++;
  }

  if (req->httpVersion == NULL) {
    req->httpVersion = HTTP_DEFAULT_VERSION;
  }
}

// Fetch the HTTP request content to setupt the request struct
void fetchRequestContent(char *token, struct request *req) {
  char *rest = token;
  char *oldToken = NULL;

  while ((token = strtok_r(rest, ": ", &rest))) {
    if (oldToken == NULL) {
      oldToken = token;
    } else {
      if (strcmp(oldToken, "Host") == 0) {
        req->host = token;
      } else if (strcmp(oldToken, "User-Agent") == 0) {
        req->userAgent = token;
      } else if (strcmp(oldToken, "Accept") == 0) {
        req->accept = token;
      } else if (strcmp(oldToken, "Connection") == 0) {
        req->connection = token;
      } else if (strcmp(oldToken, "Content-Type") == 0) {
        req->contentType = token;
      } else if (strcmp(oldToken, "Content-Length") == 0) {
        // TODO: Check if token is a valid number
      }

      // token is only one line of the request,
      // we don't continue the process
      break;
    }
  }
}

// Read a line of the HTTP request and dispatch it between 2 functions
void readRequestLine(char *token, struct request *req, bool isHeader) {
  if (isHeader == true) {
    fetchRequestHeader(token, req);
  } else {
    fetchRequestContent(token, req);
  }
}

// Check if every required parameters are given in a request
bool isValidRequest(struct request req) {
  // Check the presence of default HTTP Headers
  if (req.method == NULL || req.route == NULL || req.httpVersion == NULL) {
    return false;
  }

  // Check if the host argument is present
  if (req.host == NULL) {
    return false;
  }

  // Check if the content is well initialized (with contentType and
  // contentLegnth)
  if (req.content != NULL && req.contentLength == 0L &&
      req.contentType == NULL) {
    return false;
  }

  return true;
}

void normalizeRequestRoute(struct request *req) {
  if (req->route == NULL || req->route[0] == '\0') {
    req->route = "/";
    return;
  }

  // Route must start by /
  if (req->route[0] != '/') {
    req->route = strchr(req->route, '/');
  }
}

// Read HTTP request
void readRequest(char *buffer, struct request *req) {
  char log_message[LOG_BUFFER_SIZE];
  bool isHeader = true;
  bool isContent = false;

  // Parse Http Request
  char *token;
  char *rest = strdup(buffer);
  while ((token = strtok_r(rest, HTTP_DELIMITER, &rest))) {
    char *cp = strdup(token);
    if (strlen(token) == 0) {
      strcpy(req->content, rest);
      break;
    }

    readRequestLine(cp, req, isHeader);
    if (isHeader) {
      isHeader = false;
    }
  }

  normalizeRequestRoute(req);
  sprintf(log_message, "Request - %s %s %s", req->method, req->host,
          req->route);
  writeLog(LOG_INFO, log_message);
}
