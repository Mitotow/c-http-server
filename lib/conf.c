#include "conf.h"
#include "../server.h"
#include "../utils/strutils.h"
#include "filesystem.h"
#include "logger.h"
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Add the route to config if the route's path does not exists already
void addRouteToConfig(config_t *config, route_t *route) {
  size_t length = config->routes_size;
  for (int i = 0; i < length; i++) {
    if (strcmp(config->routes[i]->path, route->path) == 0) {
      return;
    }
  }

  length++;
  config->routes_size = length;
  config->routes =
      (route_t **)realloc(config->routes, sizeof(router_t) * length);
  config->routes[length - 1] = route;
}

// Parse and create one route
void parseRoute(config_t *config, char *strroute) {
  char *path = NULL;
  char *filename = NULL;

  strkeyval(strroute, ARRAY_SEPARATOR2, &path, &filename);

  if (path == NULL || filename == NULL) {
    if (path)
      free(path);
    if (filename)
      free(filename);
    return;
  }

  strrm(path, SPECIAL_CHAR);
  strrm(filename, SPECIAL_CHAR);

  route_t *route = (route_t *)malloc(sizeof(route_t));
  route->path = path;
  route->fileName = filename;
  addRouteToConfig(config, route);
}

// Parse a list of routes
bool parseRoutes(config_t *config, char *values) {
  uint8_t tmp = 0;
  size_t len = strlen(values);

  for (size_t i = 0; i < len; i++) {
    if ((values[i] == ARRAY_SEPARATOR &&
         (i > 0 && values[i - 1] != SPECIAL_CHAR)) ||
        i == len - 1) {
      char *strroute = strcpyft(values, tmp, i);
      writeLog(LOG_DEBUG, "Configured route : %s", strroute);
      if (!strempty(strroute)) {
        parseRoute(config, strroute);
      }
      free(strroute);
      tmp = i + 1;
    }
  }

  return true;
}

// Read and parse one line of the config file
bool parseLine(config_t *config, char line[CONFIG_LINE_BUFF]) {
  char *key, *value;
  strkeyval(line, SEPARATOR_CHAR, &key, &value);
  if (key == NULL || value == NULL) {
    return false;
  }

  if (strcmp(key, PORT_KEY) == 0) {
    uint16_t port;
    if (str_to_uint16(value, &port)) {
      config->port = port;
    } else {
      writeFatal("Invalid port %s", value);
    }
  } else if (strcmp(key, BASE_HREF_KEY) == 0) {
    config->base_href = getAbsolutePath(value);
    if (config->base_href == NULL) {
      writeFatal("Base href %s, does not exists : %d", value, errno);
    }
    config->base_href_length = strlen(config->base_href);
  } else if (strcmp(key, ROUTES_KEY) == 0) {
    parseRoutes(config, value);
  }

  return true;
}

config_t *initConfig() {
  config_t *config = (config_t *)malloc(sizeof(config_t));
  if (!config) {
    writeFatal("Cannot allocate memory for config");
  }

  config->port = DEFAULT_PORT;
  config->router_fallback = NULL;
  config->routes_size = 0;
  config->routes = NULL;
  config->base_href = DEFAULT_BASEHREF;
  config->base_href_length = strlen(DEFAULT_BASEHREF);

  return config;
}

// Parse the configuration
config_t *parseConfig() {
  config_t *config = initConfig();

  FILE *file = fopen(CONFIG_PATH, "r");
  if (!file) {
    writeLog(LOG_INFO, "Config file not found");
    return config;
  }

  char line[CONFIG_LINE_BUFF];
  while (fgets(line, sizeof(line), file)) {
    if (line[0] == COMMENTARY_CHAR)
      continue;
    parseLine(config, line);
  }

  fclose(file);
  return config;
}

void destroyConfig(config_t *config) {
  for (int i = 0; i < config->routes_size; i++) {
    free(config->routes[i]);
  }
  free(config->routes);
  free(config);
}
