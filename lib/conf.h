#include "../router.h"
#include <netinet/in.h>

#ifndef CONF_H
#define CONF_H

// Path
#define CONFIG_PATH "config"
#define CONFIG_LINE_BUFF 256

// Defaults
#define DEFAULT_BASEHREF "./public"

// Config special char
#define SEPARATOR_CHAR ":"
#define ARRAY_SEPARATOR ';'
#define ARRAY_SEPARATOR2 ","
#define COMMENTARY_CHAR '#'
#define SPECIAL_CHAR '\\'

// Config keys
#define PORT_KEY "port"
#define BASE_HREF_KEY "baseHref"
#define ROUTES_KEY "routes"

typedef struct {
  uint16_t port;
  char *router_fallback;
  size_t routes_size;
  unsigned long base_href_length;
  char *base_href;
  route_t **routes;
} config_t;

config_t *parseConfig();
uint16_t getAppPort(config_t *config);
void destroyConfig(config_t *config);

#endif
