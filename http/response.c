#include "response.h"
#include "http.h"
#include "request.h"
#include "../utils/memutils.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

// Send response to client socket
ssize_t sendResponse(int client_socket, response_t *res) {
  char header[RESPONSE_HEADER_SIZE];
  char *cur = header;
  char *const end = header + sizeof(header);

  // Add basic headers to the response
  cur += snprintf(cur, end - cur, "%s %d %s\r\n", res->httpVersion,
                  res->status.code, res->status.text);
  cur += snprintf(cur, end - cur, "Date: %s\r\n", res->date);
  cur += snprintf(cur, end - cur, "Server: %s\r\n", SERVER_NAME);
  cur += snprintf(cur, end - cur, "Connection: %s\r\n", res->connection);

  // Add parameters of keep-alive
  if (strcmp(res->connection, CONN_KEEP_ALIVE) == 0) {
    cur += snprintf(cur, end - cur, "Keep-Alive: timeout=%d, max=%d\r\n",
                    KEEP_ALIVE_TIMEOUT, KEEP_ALIVE_MAX);
  }

  // Add information about the response content
  if (res->content != NULL) {
    cur +=
        snprintf(cur, end - cur, "Content-Length: %ld\r\nContent-Type: %s\r\n",
                 res->contentLength, res->contentType);
  }

  cur += snprintf(cur, end - cur, "\r\n");

  ssize_t sent = send(client_socket, header, cur - header, 0);
  if (sent < 0) {
    return sent;
  }

  // Send the content separately if present
  if (res->content != NULL) {
    ssize_t content_sent =
        send(client_socket, res->content, res->contentLength, 0);
    if (content_sent < 0) {
      return content_sent;
    }
    sent += content_sent;
  }

  return sent;
}

// Get date for Date header
char *getDate() {
  char *date_buffer = malloc(RESPONSE_DATE_SIZE);
  time_t t;
  struct tm *gmt;

  time(&t);
  gmt = gmtime(&t);

  if (gmt == NULL) {
    free(date_buffer);
    return NULL;
  }

  strftime(date_buffer, RESPONSE_DATE_SIZE, "%a, %d %b %Y %H:%M:%S GMT", gmt);
  return date_buffer;
}

// Create basic headers of a response
void addDefaultHeadersResponse(request_t *req, response_t *res) {
  char *connection = req->connection;
  if (connection == NULL) {
    if (strcmp(req->httpVersion, HTTP_1_0)) {
      connection = CONN_CLOSE;
    } else {
      connection = CONN_KEEP_ALIVE;
    }
  }

  res->date = getDate();
  res->httpVersion = req->httpVersion;
  res->connection = connection;
}

// Create a response
response_t *createResponse(request_t *req, int statusCode) {
  response_t *res = (response_t *)calloc(1, sizeof(response_t));
  addDefaultHeadersResponse(req, res);
  const status_t *status = getStatus(statusCode);
  if (status != NULL) {
    res->status = *status;
  } else {
    res->status = *getStatus(INTERNAL_SERVER_ERROR);
  }

  return res;
}

// Create a response that contains content
response_t *createContentResponse(request_t *req, char *contentType,
                                  char *content, long length) {
  response_t *res = createResponse(req, 200);
  res->contentType = contentType;
  res->contentLength = length;
  res->content = content;

  return res;
}

void destroyResponse(response_t *res) {
  if (!res) {
    return;
  }
  
  FREE_ALL(res->date, res->content, res);
}
