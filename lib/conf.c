#include "conf.h"
#include "../server.h"
#include "../utils/strutils.h"
#include "filesystem.h"
#include "logger.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_LINE_BUFF 256

// Add the route to config if the route's path does not exists already
int addRouteToConfig(config_t *config, route_t route) {
  size_t length = config->routes_size;
  for (int i = 0; i < length; i++) {
    if (strcmp(config->routes[i].path, route.path) == 0) {
      return -1;
    }
  }

  length++;
  config->routes =
      (route_t *)realloc(config->routes, sizeof(router_t) * length);
  config->routes[length - 1] = route;
  config->routes_size = length;

  return 0;
}

// Read and parse one line of the config file
bool parseLine(config_t *config, char line[CONFIG_LINE_BUFF]) {
  char *key = strtok_r(line, ":", &line);
  if (key == NULL)
    return false;
  key = trimOnlySpaceAndTab(key);
  char *value = strtok_r(NULL, "", &line);
  if (value == NULL)
    return false;
  value = trimOnlySpaceAndTab(value);

  if (strcmp(key, PORT_KEY) == 0) {
    uint16_t port;
    if (str_to_uint16(value, &port)) {
      config->port = port;
    } else {
      writeFatal("Invalid port %s", value);
    }
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
    parseLine(config, line);
  }

  fclose(file);
  return config;
}

void destroyConfig(config_t *config) {
  free(config->routes);
  free(config);
}
