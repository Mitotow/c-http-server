// Server side C program to demonstrate Socket programming
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

// HTTP Parameters
#define HTTP_DELIMITER "\r\n"
#define HTTP_REQ_BUFFER_SIZE 4096
#define HTTP_MAX_HEADER_SIZE 512
#define HTTP_DEFAULT_VERSION "HTTP/1.1"
#define HTTP_1_0 "HTTP/1.0"

// HTTP Methods
#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_PUT "PUT"
#define HTTP_DELETE "DELETE"

// File
#define BASE_DIR "."
#define EXT_ICO "ico"
#define EXT_HTML "html"

// Content-Type
#define CTYPE_HTML "text/html"
#define CTYPE_ICO "image/x-icon"

// Connection
#define CONN_CLOSE "close"
#define CONN_KEEP_ALIVE "keep-alive"

// Keep-Alive
#define KEEP_ALIVE_TIMEOUT 5 // 5 seconds before timeout
#define KEEP_ALIVE_MAX 100   // Max requests per session

// Logging
#define LOG_INFO "INFO"
#define LOG_TRACE "TRACE"
#define LOG_DEBUG "DEBUG"
#define LOG_WARN "WARN"
#define LOG_ERROR "ERROR"
#define LOG_FATAL "FATAL"
#define LOG_BUFFER_SIZE 1024

static int server_fd;

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

struct response {
  char *httpVersion;
  int statusCode;
  char *statusName;
  long contentLength;
  char *contentType;
  char *connection;
  char *content;
};

struct route {
  char *path;
  char *file;
};

// Write log in console with a basic format
void writeLog(char *level, char *message) {
  if ((strcmp(level, LOG_DEBUG) == 0) && !DEBUG) {
    return;
  }
  size_t s_buff = strlen(level) + 1 + strlen(message) + 1;
  char buff[s_buff];
  sprintf(buff, "[%s] %s", level, message);
  if (strcmp(level, LOG_FATAL) == 0 || strcmp(level, LOG_ERROR) == 0) {
    perror(buff);
  } else {
    printf("%s\n", buff);
  }
}

// Write log in console with a basic format including the socket value
// Mainely use to debug the server
void writeLogSocket(char *level, char *message, int socket) {
  size_t s_buff = strlen(message) + 51;
  char buff[s_buff];
  sprintf(buff, "[client %d] %s", socket, message);
  writeLog(level, buff);
}

// Intercept CTRL+C and close the server's file descriptor
void closeOnSignal(int sig) {
  char *log_message = "Server stopped";
  signal(sig, SIG_IGN);
  close(server_fd);
  writeLog(LOG_INFO, log_message);
  exit(EXIT_SUCCESS);
}

// Retrieve file path from a request
char *getFilePathFromRequest(struct request req) {
  char *path;
  size_t size = strlen(BASE_DIR) + 1 + strlen(req.route) + 1;
  path = (char *)malloc(size);

  strcpy(path, BASE_DIR);
  strcat(path, req.route);

  return path;
}

// Read file, return it's content and set the s
// parameter as the size of the file
char *readFile(const char *ctype, const char *path, long *s) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    return NULL;
  }

  // Retrieve file size
  fseek(file, 0, SEEK_END);
  *s = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Alloc buffer for the content
  char *buffer = (char *)malloc(*s);
  if (!buffer) {
    writeLog(LOG_ERROR, "Cannot allocate memory to read file");
    fclose(file);
    return NULL;
  }

  // Read file content
  size_t bytes_read = fread(buffer, 1, *s, file);
  if (bytes_read != (size_t)*s) {
    writeLog(LOG_ERROR, "Error while reading a file");
    free(buffer);
    fclose(file);
    return NULL;
  }

  if (strcmp(ctype, CTYPE_HTML) == 0) {
    char *txt_buffer = (char *)realloc(buffer, *s + 1);
    if (!txt_buffer) {
      writeLog(LOG_ERROR, "Problem when trying to realloc buffer");
      free(buffer);
      fclose(file);
      return NULL;
    }
    buffer = txt_buffer;
    buffer[*s] = '\0';
  }

  fclose(file);
  return buffer;
}

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
  if (req.host == false) {
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

// Send response to the client
ssize_t sendResponse(int client_socket, struct response res) {
  char header[HTTP_MAX_HEADER_SIZE];
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
void addDefaultHeadersResponse(struct request req, struct response *res) {
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
void createResponse(struct request req, struct response *res, int statusCode,
                    char *statusName) {
  addDefaultHeadersResponse(req, res);
  res->statusCode = statusCode;
  res->statusName = statusName;
}

// Create a response that contains content
void createContentResponse(struct request req, struct response *res,
                           char *contentType, char *content, long length) {
  createResponse(req, res, 200, "OK");
  res->contentType = contentType;
  res->contentLength = length;
  res->content = content;
}

// Create a not found response
void createNotFoundResponse(struct request req, struct response *res) {
  createResponse(req, res, 404, "Not Found");
}

// Create a bad request response
void createBadRequestResponse(struct request req, struct response *res) {
  createResponse(req, res, 400, "Bad Request");
}

// Retrieve the extension of the file of a route, return NULL if the route
// does not point to a specific file
char *getExtension(const char *route) {
  char *pt = strrchr(route, '.');
  if (pt == NULL) {
    return NULL;
  } else {
    return pt + 1;
  }
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
void handleGetFile(struct request req, struct response *res) {
  char *filePath = getFilePathFromRequest(req);
  if (filePath == NULL) {
    createNotFoundResponse(req, res);
  } else {
    char *contentType =
        (strcmp(req.route, EXT_ICO) == 0) ? CTYPE_ICO : CTYPE_HTML;
    long contentSize;
    char *content = readFile(contentType, filePath, &contentSize);
    free(filePath);

    if (content != NULL) {
      createContentResponse(req, res, contentType, content, contentSize);
    } else {
      createNotFoundResponse(req, res);
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

  do {
    char buffer[HTTP_REQ_BUFFER_SIZE] = {0};
    ssize_t bytes_read;

    writeLogSocket(LOG_DEBUG, "Reading client socket", client_socket);
    bytes_read = read(client_socket, buffer, sizeof(buffer));
    if (bytes_read < 0) {
      // EAGAIN or EWOULDBLOCK = timeout
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        writeLogSocket(LOG_ERROR, "Reading error", client_socket);
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
      createBadRequestResponse(req, &res);
      sendResponse(client_socket, res);
      writeLogSocket(LOG_DEBUG, "Bad Request", client_socket);
      memset(buffer, 0, HTTP_REQ_BUFFER_SIZE);
      continue;
    }

    writeLogSocket(LOG_DEBUG,
                   "Get the requested file path using the request route",
                   client_socket);

    // ONLY FOR GET REQUEST
    if (strcmp(req.method, HTTP_GET) == 0) {
      char *ext = getExtension(req.route);
      if (ext != NULL) {
        handleGetFile(req, &res);
      } else {
        // TODO: Faire un fonctionnement de router
        createBadRequestResponse(req, &res);
      }
    } else {
      // We only handle GET request with this server
      createBadRequestResponse(req, &res);
    }

    writeLogSocket(LOG_DEBUG, "Send response to client", client_socket);
    size_t bytes_sent;
    if ((bytes_sent = sendResponse(client_socket, res)) < 0) {
      writeLogSocket(LOG_ERROR, "Error sending to client", client_socket);
    } else {
      char log_message[LOG_BUFFER_SIZE];
      sprintf(log_message,
              "Response - version=%s status=%d-%s type=%s content-length=%ld "
              "sent=%ld",
              res.httpVersion, res.statusCode, res.statusName, res.contentType,
              res.contentLength, bytes_sent);
      writeLogSocket(LOG_INFO, log_message, client_socket);
    }

    writeLogSocket(LOG_DEBUG, "Check the connection state in the request",
                   client_socket);

    if (strcmp(res.connection, CONN_CLOSE) == 0) {
      break;
    }

    memset(buffer, 0, HTTP_REQ_BUFFER_SIZE);
  } while (true);

  writeLogSocket(LOG_INFO, "Disconnected", client_socket);
  close(client_socket);
  free(argument);
  return thrd_success;
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

  do {
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
      writeLogSocket(LOG_ERROR, "Cannot create thread", *client_fd_ptr);
      close(*client_fd_ptr);
      free(client_fd_ptr);
      continue;
    }

    thrd_detach(id_thread);
  } while (true);

  return 0;
}
