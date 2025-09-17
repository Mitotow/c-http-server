#include "router.h"
#include "logger.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct HTTP_router *router;

void initRouter() {
  router = (struct HTTP_router *)malloc(sizeof(size_t));
  router->routes_size = 0;
}

bool existsRoute(struct HTTP_route route) {
  for (int i = 0; i < router->routes_size; i++) {
    if (router->routes[i].path == route.path) {
      return true;
    }
  }

  return false;
}

void addRoute(char *path, char *filename) {
  size_t length = router->routes_size;

  struct HTTP_route route;
  route.path = path;
  route.fileName = filename;

  if (existsRoute(route))
    return;
  length++;
  router = (struct HTTP_router *)realloc(
      router, sizeof(size_t) + length * sizeof(struct HTTP_route));
  router->routes_size = length;
  router->routes[length - 1] = route;
}

char *getFilename(char *path) {
  for (int i = 0; i < router->routes_size; i++) {
    if (strcmp(router->routes[i].path, path) == 0) {
      return router->routes[i].fileName;
    }
  }

  writeLog(LOG_DEBUG, "Could not find a proper route");

  return NULL;
}

void destroyRouter() { free(router); }
