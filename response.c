#include "response.h"
#include "http.h"
#include "request.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

ssize_t sendResponse(int client_socket, response_t res) {
  char header[RESPONSE_HEADER_SIZE];
  char *cur = header;
  char *const end = header + sizeof(header);

  // Add basic headers to the response
  cur += snprintf(cur, end - cur, "%s %d %s\r\n", res.httpVersion,
                  res.statusCode, res.statusName);
  cur += snprintf(cur, end - cur, "Connection: %s\r\n", res.connection);

  // Add parameters of keep-alive
  if (strcmp(res.connection, CONN_KEEP_ALIVE) == 0) {
    cur += snprintf(cur, end - cur, "Keep-Alive: timeout=%d, max=%d\r\n",
                    KEEP_ALIVE_TIMEOUT, KEEP_ALIVE_MAX);
  }

  // Add information about the response content
  if (res.content != NULL) {
    cur +=
        snprintf(cur, end - cur, "Content-Length: %ld\r\nContent-Type: %s\r\n",
                 res.contentLength, res.contentType);
  }

  cur += snprintf(cur, end - cur, "\r\n");

  ssize_t sent = send(client_socket, header, cur - header, 0);
  if (sent < 0) {
    return sent;
  }

  // Send the content separately if present
  if (res.content != NULL) {
    ssize_t content_sent =
        send(client_socket, res.content, res.contentLength, 0);
    if (content_sent < 0) {
      return content_sent;
    }
    sent += content_sent;
  }

  return sent;
}

// Create basic headers of a response
void addDefaultHeadersResponse(request_t req, response_t *res) {
  char *connection = req.connection;
  if (connection == NULL) {
    if (strcmp(req.httpVersion, HTTP_1_0)) {
      connection = CONN_CLOSE;
    } else {
      connection = CONN_KEEP_ALIVE;
    }
  }

  // TODO : Add the Date header in the response

  res->httpVersion = req.httpVersion;
  res->connection = connection;
}

// Create a response
void createResponse(request_t req, response_t *res, int statusCode) {
  addDefaultHeadersResponse(req, res);
  res->statusCode = statusCode;
}

// Create a response that contains content
void createContentResponse(request_t req, response_t *res, char *contentType,
                           char *content, long length) {
  createResponse(req, res, 200);
  res->contentType = contentType;
  res->contentLength = length;
  res->content = content;
}
