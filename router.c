#include "router.h"
#include "logger.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Init router
router_t *initRouter() {
  router_t *router = (router_t *)malloc(sizeof(router_t));
  router->routes_size = 0;
  router->routes = (route_t *)malloc(sizeof(route_t) * router->routes_size);

  return router;
}

// Check if a route exists
bool existsRoute(router_t *router, route_t route) {
  for (int i = 0; i < router->routes_size; i++) {
    if (router->routes[i].path == route.path) {
      return true;
    }
  }

  return false;
}

// Add route to router
void addRoute(router_t *router, char *path, char *filename) {
  size_t length = router->routes_size;

  route_t route;
  route.path = path;
  route.fileName = filename;

  if (existsRoute(router, route))
    return;
  length++;
  router->routes_size = length;
  router->routes =
      (route_t *)realloc(router->routes, sizeof(router_t) * length);
  router->routes[length - 1] = route;
}

// Return the route by giving the path
route_t *getRouteByPath(router_t *router, char *path) {
  for (int i = 0; i < router->routes_size; i++) {
    if (strcmp(router->routes[i].path, path) == 0) {
      return &router->routes[i];
    }
  }

  writeLog(LOG_DEBUG, "Could not find a proper route");

  return NULL;
}

// Destroy router
void destroyRouter(router_t *router) {
  free(router->routes);
  free(router);
}
