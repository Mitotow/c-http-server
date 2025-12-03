#include "lib/conf.h"
#include "lib/filesystem.h"
#include "router.h"
#include "server.h"
#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>

static server_context_t *ctx;

// Handle signal
void handleSignal(int sig) {
  signal(sig, SIG_IGN);

  if (ctx != NULL) {
    stopServer(ctx);
  }
}

int main() {
  config_t *config = parseConfig();
  ctx = createServer(config);
  setRouter(ctx, initRouter(config->routes, config->routes_size));
  runServer(ctx);
  signal(SIGINT, handleSignal);
  return 0;
}
