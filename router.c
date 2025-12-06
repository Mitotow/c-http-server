#include "router.h"
#include "lib/conf.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Init router
router_t *initRouter(route_t **routes, size_t routes_size) {
  router_t *router = (router_t *)malloc(sizeof(router_t));
  router->routes_size = routes_size;
  router->routes = routes;

  return router;
}

// Check if a route exists
bool existsRoute(router_t *router, route_t route) {
  for (int i = 0; i < router->routes_size; i++) {
    if (strcmp(router->routes[i]->path, route.path) == 0) {
      return true;
    }
  }

  return false;
}

// Return the route by giving the path
route_t *getRouteByPath(router_t *router, char *path) {
  for (int i = 0; i < router->routes_size; i++) {
    if (strcmp(router->routes[i]->path, path) == 0) {
      return router->routes[i];
    }
  }

  return NULL;
}

// Destroy router
void destroyRouter(router_t *router) {
  free(router->routes);
  free(router);
}
