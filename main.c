#include "router.h"
#include "server.h"
#include <signal.h>

static server_context_t *ctx;

// Handle signal
void handleSignal(int sig) {
  signal(sig, SIG_IGN);

  if (ctx != NULL) {
    stopServer(ctx);
  }
}

// Create the server's router
router_t *createRouter() {
  router_t *router = initRouter();
  addRoute(router, "/", "index.html");
  addRoute(router, "/hello", "hello.html");
  addRoute(router, "/test", "test/test.html");

  return router;
}

int main() {
  ctx = createServer();
  setRouter(ctx, createRouter());
  runServer(ctx);

  signal(SIGINT, handleSignal);

  return 0;
}
