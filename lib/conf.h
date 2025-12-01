#include "../router.h"
#include <netinet/in.h>

#ifndef CONF_H
#define CONF_H

// Path
#define CONFIG_PATH "config"

// Config keys
#define PORT_KEY "port"

typedef struct {
  uint16_t port;
  char *router_fallback;
  size_t routes_size;
  route_t *routes;
} config_t;

config_t *parseConfig();
uint16_t getAppPort(config_t *config);
void destroyConfig(config_t *config);

#endif
