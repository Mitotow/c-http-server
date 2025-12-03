#include "lib/conf.h"
#include "router.h"
#include <netinet/in.h>
#include <sys/socket.h>

#ifndef SERVER_H
#define SERVER_H

// Server Parameters
#define SERVER_NAME "CustomAbrst"
#define DEFAULT_PORT 8080
#define DEBUG true

typedef struct {
  int server_fd;
  char *static_dir;
  struct sockaddr address;
  socklen_t addrlen;
  router_t *router;
  config_t *config;
} server_context_t;

typedef struct {
  int client_fd;
  server_context_t *ctx;
} handle_client_argument_t;

server_context_t *createServer(config_t *config);
void setRouter(server_context_t *ctx, router_t *router);
void runServer(server_context_t *ctx);
void stopServer(server_context_t *ctx);

#endif
