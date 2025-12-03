#include "http/request.h"
#include <stdlib.h>

#ifndef ROUTER_H
#define ROUTER_H

typedef struct {
  char *path;
  char *fileName;
} route_t;

typedef struct {
  size_t routes_size;
  route_t **routes;
} router_t;

router_t *initRouter(route_t **routes, size_t routes_size);
route_t *getRouteByPath(router_t *router, char *path);
void destroyRouter(router_t *router);

#endif
