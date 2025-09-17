#include "fm.h"
#include "http.h"
#include "logger.h"
#include "request.h"
#include "response.h"
#include "router.h"
#include <errno.h>
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
#define DEFAULT_PORT 8081
#define DEBUG true

static int server_fd;

// Intercept CTRL+C and close the server's file descriptor
void closeOnSignal(int sig) {
  char *log_message = "Server stopped";
  signal(sig, SIG_IGN);
  close(server_fd);
  writeLog(LOG_INFO, log_message);
  exit(EXIT_SUCCESS);
}

// Return the corresponding content type
char *getContentType(const char *ext) {
  if (strcmp(ext, EXT_ICO) == 0) {
    return CTYPE_ICO;
  } else if (strcmp(ext, EXT_HTML) == 0) {
    return CTYPE_HTML;
  }

  return NULL;
}

// Function in charge of retrieving the file and create the response
void handleGetFile(struct request req, struct response *res, char *filePath) {
  if (filePath == NULL) {
    createResponse(req, res, NOT_FOUND);
  } else {
    char *contentType =
        (strcmp(req.route, EXT_ICO) == 0) ? CTYPE_ICO : CTYPE_HTML;
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
static int handleClient(void *argument) {
  int client_socket = *(int *)argument;

  // Setup timeout
  struct timeval tv;
  tv.tv_sec = KEEP_ALIVE_TIMEOUT;
  tv.tv_usec = 0;
  setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  while (true) {
    char buffer[HTTP_REQ_BUFFER_SIZE] = {0};
    ssize_t bytes_read;

    writeLogSocket(LOG_DEBUG, client_socket, "Reading client socket");
    bytes_read = read(client_socket, buffer, sizeof(buffer));
    if (bytes_read < 0) {
      // EAGAIN or EWOULDBLOCK = timeout
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        writeLogSocket(LOG_ERROR, client_socket, "Reading error");
      }

      break;
    } else if (bytes_read == 0) {
      break;
    }

    struct request req;
    struct response res;
    writeLog(LOG_DEBUG, "Read the retrieved data and setup the request struct");
    readRequest(buffer, &req);
    if (!isValidRequest(req)) {
      writeLogSocket(LOG_DEBUG, client_socket, "Invalid request");
      createResponse(req, &res, BAD_REQUEST);
      sendResponse(client_socket, res);
      writeLogSocket(LOG_DEBUG, client_socket, "Bad Request");
      memset(buffer, 0, HTTP_REQ_BUFFER_SIZE);
      continue;
    }

    // ONLY FOR GET REQUEST
    if (strcmp(req.method, HTTP_GET) == 0) {
      char *ext = getExtension(req.route);
      if (ext != NULL) {
        char *filePath = getFilePathFromRequest(req);
        if (filePath == NULL) {
          createResponse(req, &res, NOT_FOUND);
        } else {
          handleGetFile(req, &res, filePath);
          free(filePath);
        }
      } else {
        char *filename;
        if ((filename = getFilename(req.route)) == NULL) {
          createResponse(req, &res, BAD_REQUEST);
        } else {
          handleGetFile(req, &res, filename);
        }
      }
    } else {
      // We only handle GET request with this server
      createResponse(req, &res, BAD_REQUEST);
    }

    writeLogSocket(LOG_DEBUG, client_socket, "Send response to client");
    size_t bytes_sent;
    if ((bytes_sent = sendResponse(client_socket, res)) < 0) {
      writeLogSocket(LOG_ERROR, client_socket, "Error sending to client");
    } else {
      char log_message[LOG_BUFFER_SIZE];
      sprintf(log_message,
              "Response - version=%s status=%d-%s type=%s content-length=%ld "
              "sent=%ld",
              res.httpVersion, res.statusCode, res.statusName, res.contentType,
              res.contentLength, bytes_sent);
      writeLogSocket(LOG_INFO, client_socket, log_message);
    }

    writeLogSocket(LOG_DEBUG, client_socket,
                   "Check the connection state in the request");

    if (strcmp(res.connection, CONN_CLOSE) == 0) {
      break;
    }

    memset(buffer, 0, HTTP_REQ_BUFFER_SIZE);
  }

  writeLogSocket(LOG_INFO, client_socket, "Client disconnected");
  close(client_socket);
  free(argument);
  return thrd_success;
}

void createRoutes() {
  initRouter();
  addRoute("/", "./index.html");
}

int main(int argc, char const *argv[]) {
  int server_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    writeLog(LOG_FATAL, "Cannot create socket");
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, closeOnSignal);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(DEFAULT_PORT);

  memset(address.sin_zero, '\0', sizeof address.sin_zero);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    writeLog(LOG_FATAL, "Cannot bind address to socket");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 10) < 0) {
    writeLog(LOG_FATAL, "Cannot listen to port");
    exit(EXIT_FAILURE);
  }

  char log_message[LOG_BUFFER_SIZE];
  sprintf(log_message, "Server listening on port %d", DEFAULT_PORT);
  writeLog(LOG_INFO, log_message);

  createRoutes();

  while (true) {
    writeLog(LOG_DEBUG, "Allocating memory for client socket pointer");
    int *client_fd_ptr = malloc(sizeof(int));
    if (!client_fd_ptr) {
      writeLog(LOG_ERROR, "Cannot allocate memory for client socket");
      continue;
    }

    writeLog(LOG_DEBUG, "Waiting for connections");
    if ((*client_fd_ptr = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0) {
      writeLog(LOG_ERROR, "Cannot accept connection from a client");
      free(client_fd_ptr);
      continue;
    }

    thrd_t id_thread;
    writeLog(LOG_DEBUG, "Creating thread to handle new client");
    if (thrd_create(&id_thread, &handleClient, (void *)client_fd_ptr) !=
        thrd_success) {
      writeLogSocket(LOG_ERROR, *client_fd_ptr, "Cannot create thread");
      close(*client_fd_ptr);
      free(client_fd_ptr);
      continue;
    }

    thrd_detach(id_thread);
  }

  return 0;
}
