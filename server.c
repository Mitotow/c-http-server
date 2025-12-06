#include "server.h"
#include "http/http.h"
#include "http/request.h"
#include "http/response.h"
#include "lib/filesystem.h"
#include "lib/logger.h"
#include "router.h"
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <threads.h>
#include <unistd.h>

// Function in charge of retrieving the file and create the response
response_t *handleGet(request_t *req, char *filePath, bool isHead) {
  response_t *res;
  if (filePath == NULL) {
    res = createResponse(req, NOT_FOUND);
  } else {
    char *ext = getExtension(filePath);
    char *contentType = (char *)getContentType(ext);
    long contentSize;

    if (isHead) {
      contentSize = getFileSizeFromPath(filePath);
      if (contentSize == -1) {
        res = createResponse(req, NOT_FOUND);
      } else {
        res = createContentResponse(req, contentType, NULL, contentSize);
      }
      return res;
    }

    char *content;
    if (isTextContentType(contentType)) {
      content = readTextFile(filePath, &contentSize);
    } else {
      content = readFile(filePath, &contentSize);
    }

    if (content != NULL && contentSize > 0) {
      res = createContentResponse(req, contentType, content, contentSize);
    } else {
      res = createResponse(req, NOT_FOUND);
    }
  }

  return res;
}

// Retrieve the file path from the request or router if needed
char *getFilePathFromRequest(server_context_t *ctx, request_t req) {
  char *ext = getExtension(req.route);
  if (ext != NULL) {
    // Request a direct file
    writeLog(LOG_DEBUG, "Request a direct file");
    return getFilePath(ctx, req.route);
  } else if (ctx->router != NULL) {
    // Request a configured route
    // TODO: Implement an handler system for each route
    writeLog(LOG_DEBUG, "Request a configured route");
    route_t *route;
    if ((route = getRouteByPath(ctx->router, req.route)) != NULL) {
      return getFilePath(ctx, route->fileName);
    }
  }

  return NULL;
}

// Handle request from client
response_t *handleRequest(handle_client_argument_t *arg, request_t *req) {
  bool isHead = strcmp(req->method, HTTP_HEAD) == 0;
  // ONLY FOR GET AND HEAD REQUEST
  response_t *res;
  if (strcmp(req->method, HTTP_GET) == 0 || isHead) {
    char *filePath = getFilePathFromRequest(arg->ctx, *req);

    if (!is_valid_path(arg->ctx, filePath)) {
      res = createResponse(req, FORBIDDEN);
    } else if (filePath != NULL) {
      res = handleGet(req, filePath, isHead);
    } else {
      if (arg->ctx->config->fallback) {
        free(req->route);
        req->route = strdup(arg->ctx->config->fallback);
        res = handleRequest(arg, req);
      } else {
        // Route not found
        res = createResponse(req, NOT_FOUND);
      }
    }

    if (filePath)
      free(filePath);
  } else {
    // We only handle GET request actually
    res = createResponse(req, BAD_REQUEST);
  }

  return res;
}

// Handle communication between server and one client
int handleClient(void *argument) {
  handle_client_argument_t *arg = (handle_client_argument_t *)argument;

  // Setup timeout
  struct timeval tv;
  tv.tv_sec = KEEP_ALIVE_TIMEOUT;
  tv.tv_usec = 0;

  while (1) {
    char buffer[HTTP_REQ_BUFFER_SIZE] = {0};
    ssize_t bytes_read;

    setsockopt(arg->client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    bytes_read = recv(arg->client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
      if (bytes_read == 0) {
        writeLog(LOG_DEBUG, "Client close connection");
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        writeLog(LOG_DEBUG, "Connection timed out");
      } else {
        writeLog(LOG_ERROR, "Read error");
      }

      break;
    }

    // Fetch request content
    writeLog(LOG_DEBUG, "Read the retrieved data and setup the request struct");
    request_t *req = createRequest(buffer);
    memset(buffer, 0, HTTP_REQ_BUFFER_SIZE);
    if (!isValidRequest(req)) {
      writeLog(LOG_DEBUG, "Invalid request");
      response_t *res = createResponse(req, BAD_REQUEST);
      sendResponse(arg->client_fd, res);
      destroyRequest(req);
      destroyResponse(res);
      continue;
    }

    writeLog(LOG_INFO, "Request - method=%s conn=%s host=%s route=%s",
             req->method, req->connection, req->host, req->route);

    response_t *res = handleRequest(arg, req);

    bool isClosed =
        req->connection == NULL || strcmp(req->connection, CONN_CLOSE) == 0;

    // SEND RESPONSE TO CLIENT
    size_t bytes_sent;
    if ((bytes_sent = sendResponse(arg->client_fd, res)) < 0) {
      writeLog(LOG_ERROR, "Response FAILED - Requested route = %s", req->route);
    } else {
      writeLog(LOG_INFO,
               "Response - version=%s status=%d-%s type=%s content-length=%ld "
               "sent=%ld",
               res->httpVersion, res->status.code, res->status.text,
               res->contentType, res->contentLength, bytes_sent);
    }

    destroyRequest(req);
    destroyResponse(res);

    if (isClosed)
      break;
  }

  writeLog(LOG_DEBUG, "Client disconnected");
  close(arg->client_fd);
  free(arg);
  return thrd_success;
}

// Handle incoming connections
void handleConnections(server_context_t *ctx) {
  while (1) {
    int client_fd;

    writeLog(LOG_INFO, "Waiting for connections");
    if ((client_fd = accept(ctx->server_fd, &ctx->address, &ctx->addrlen)) <
        0) {
      continue;
    }

    handle_client_argument_t *arg = malloc(sizeof(handle_client_argument_t));
    arg->client_fd = client_fd;
    arg->ctx = ctx;

    thrd_t id_thread;
    writeLog(LOG_DEBUG, "Creating thread to handle new client");
    if (thrd_create(&id_thread, &handleClient, (void *)arg) != thrd_success) {
      close(client_fd);
      free(arg);
      continue;
    }

    thrd_detach(id_thread);
  }
}

// Create a server context
server_context_t *createServer(config_t *config) {
  int server_fd;
  struct sockaddr_in address;
  uint16_t port = config->port;

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    writeFatal("Cannot create socket");
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  memset(address.sin_zero, '\0', sizeof address.sin_zero);

  server_context_t *ctx = (server_context_t *)malloc(sizeof(server_context_t));
  ctx->server_fd = server_fd;
  ctx->static_dir = PUBLIC_DIR;
  ctx->address = *(struct sockaddr *)&address;
  ctx->addrlen = sizeof(address);
  ctx->config = config;

  return ctx;
}

int bindServer(server_context_t *ctx) {
  return bind(ctx->server_fd, &ctx->address, sizeof(ctx->address));
}

// Set the server router
void setRouter(server_context_t *ctx, router_t *router) {
  if (bindServer(ctx) < 0) {
    writeFatal("Cannot bind server");
  }

  ctx->router = router;
}

// Make the server listening and run the connection handler
void runServer(server_context_t *ctx) {
  if (listen(ctx->server_fd, SOMAXCONN) < 0) {
    writeFatal("Cannot listen");
  }

  writeLog(LOG_INFO, "Server listening on port %d", ctx->config->port);

  handleConnections(ctx);
}

// Stop server
void stopServer(server_context_t *ctx) {
  close(ctx->server_fd);
  if (ctx->router != NULL) {
    destroyRouter(ctx->router);
  }
  free(ctx);

  writeLog(LOG_INFO, "Server stopped");
}
