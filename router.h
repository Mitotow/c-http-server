#ifndef ROUTER_H
#define ROUTER_H

#include <stdlib.h>

typedef struct {
  char *path;
  char *fileName;
} route_t;

typedef struct {
  size_t routes_size;
  route_t *routes;
} router_t;

router_t *initRouter();
void addRoute(router_t *router, char *path, char *filename);
route_t *getRouteByPath(router_t *router, char *path);
void destroyRouter(router_t *router);

#endif
