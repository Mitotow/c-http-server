#include <stdlib.h>
#ifndef ROUTER_H
#define ROUTER_H

struct HTTP_route {
  char *path;
  char *fileName;
};

struct HTTP_router {
  size_t routes_size;
  struct HTTP_route routes[];
};

void initRouter();
void addRoute(char *path, char *filename);
char *getFilename(char *path);
void destroyRouter();

#endif
