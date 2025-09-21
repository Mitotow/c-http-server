#include "fm.h"
#include "http.h"
#include "logger.h"
#include "request.h"
#include "response.h"
#include "router.h"
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <threads.h>
#include <unistd.h>

// Server Parameters
#define DEFAULT_PORT 8080
#define DEBUG true

static server_context_t ctx;

// Intercept CTRL+C and close the server's file descriptor
void closeOnSignal(int sig) {
  signal(sig, SIG_IGN);
  close(ctx.server_fd);
  destroyRouter(ctx.router);
  writeLog(LOG_INFO, "Server stopped");
  exit(EXIT_SUCCESS);
}

// Return the corresponding content type
char *getContentType(const char *ext) {
  if (strcmp(ext, EXT_HTML) == 0) {
    return CTYPE_HTML;
  } else if (strcmp(ext, EXT_JS) == 0) {
    return CTYPE_JS;
  } else if (strcmp(ext, EXT_ICO) == 0) {
    return CTYPE_ICO;
  }

  return NULL;
}

// Function in charge of retrieving the file and create the response
void handleGetFile(request_t req, response_t *res, char *filePath) {
  if (filePath == NULL) {
    createResponse(req, res, NOT_FOUND);
  } else {
    char *ext = getExtension(filePath);
    if (ext == NULL) {
      createResponse(req, res, NOT_FOUND);
      return;
    }

    char *contentType = getContentType(ext);
    if (contentType == NULL) {
      createResponse(req, res, NOT_FOUND);
      return;
    }

    long contentSize;
    char *content = readFile(contentType, filePath, &contentSize);

    if (content != NULL) {
      createContentResponse(req, res, contentType, content, contentSize);
    } else {
      createResponse(req, res, NOT_FOUND);
    }
  }
}

// Handle communication between server and one client
int handleClient(void *argument) {
  int client_socket = (int)(long)argument;

  // Setup timeout
  struct timeval tv;
  tv.tv_sec = KEEP_ALIVE_TIMEOUT;
  tv.tv_usec = 0;
  setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  writeLog(LOG_DEBUG, "Handle Client");

  while (1) {
    char buffer[HTTP_REQ_BUFFER_SIZE] = {0};
    ssize_t bytes_read;

    bytes_read = read(client_socket, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
      break;
    }

    request_t req;
    response_t res;
    writeLog(LOG_DEBUG, "Read the retrieved data and setup the request struct");
    readRequest(buffer, &req);
    if (!isValidRequest(req)) {
      writeLog(LOG_DEBUG, "Invalid request");
      createResponse(req, &res, BAD_REQUEST);
      sendResponse(client_socket, res);
      memset(buffer, 0, HTTP_REQ_BUFFER_SIZE);
      continue;
    }

    // ONLY FOR GET REQUEST
    if (strcmp(req.method, HTTP_GET) == 0) {
      char *ext = getExtension(req.route);
      char *filePath;
      if (ext != NULL) {
        // Request a direct file
        writeLog(LOG_DEBUG, "Request a direct file");
        filePath = getFilePathFromRequest(req);
      } else {
        // Request a configured route
        // TODO: Implement an handler system in all routes
        writeLog(LOG_DEBUG, "Request a configured route");
        route_t *route;
        if ((route = getRouteByPath(ctx.router, req.route)) != NULL) {
          filePath = getFilePathFromRoute(*route);
        }
      }

      if (filePath != NULL) {
        handleGetFile(req, &res, filePath);
        free(filePath);
      } else {
        createResponse(req, &res, NOT_FOUND);
      }
    } else {
      // We only handle GET request with this server
      createResponse(req, &res, BAD_REQUEST);
    }

    size_t bytes_sent;
    if ((bytes_sent = sendResponse(client_socket, res)) < 0) {
    } else {
      writeLog(LOG_INFO,
               "Response - version=%s status=%d-%s type=%s content-length=%ld "
               "sent=%ld",
               res.httpVersion, res.statusCode, res.statusName, res.contentType,
               res.contentLength, bytes_sent);
    }

    memset(buffer, 0, HTTP_REQ_BUFFER_SIZE);

    if (strcmp(res.connection, CONN_CLOSE) == 0) {
      break;
    }
  }

  writeLog(LOG_DEBUG, "Client disconnected");
  close(client_socket);
  return thrd_success;
}

router_t *createRouter() {
  router_t *router = initRouter();
  addRoute(router, "/", "index.html");

  return router;
}

void handleConnections() {
  while (1) {
    int client_fd;

    writeLog(LOG_DEBUG, "Waiting for connections");
    if ((client_fd = accept(ctx.server_fd, ctx.address, ctx.addrlen)) < 0) {
      continue;
    }

    thrd_t id_thread;
    writeLog(LOG_DEBUG, "Creating thread to handle new client");
    if (thrd_create(&id_thread, &handleClient, (void *)(long)client_fd) !=
        thrd_success) {
      close(client_fd);
      continue;
    }

    writeLog(LOG_INFO, "Handling new client");

    thrd_detach(id_thread);
  }
}

int main(int argc, char const *argv[]) {
  int server_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    writeFatal("Cannot create socket");
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(DEFAULT_PORT);

  memset(address.sin_zero, '\0', sizeof address.sin_zero);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    writeFatal("Cannot bind address to socket");
  }

  router_t *router = createRouter();

  if (listen(server_fd, 10) < 0) {
    writeFatal("Cannot listen to port");
  }

  writeLog(LOG_INFO, "Server listening on port %d", DEFAULT_PORT);

  ctx.server_fd = server_fd;
  ctx.static_dir = PUBLIC_DIR;
  ctx.address = (struct sockaddr *)&address;
  ctx.addrlen = (socklen_t *)&addrlen;
  ctx.router = router;

  // Handle signal like CTRL+C after the initialization of server ctx
  signal(SIGINT, closeOnSignal);

  handleConnections();

  return 0;
}
