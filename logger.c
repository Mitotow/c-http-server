#include "logger.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <threads.h>
#include <unistd.h>

// Write log in console with a basic format
void writeLog(char *level, char *message) {
  size_t s_buff = strlen(level) + 1 + strlen(message) + 1;
  char buff[s_buff];
  sprintf(buff, "[%s] %s", level, message);
  if (strcmp(level, LOG_FATAL) == 0 || strcmp(level, LOG_ERROR) == 0) {
    perror(buff);
  } else {
    printf("%s\n", buff);
  }
}

// Write log in console with a basic format including the socket value
// Mainely use to debug the server
void writeLogSocket(char *level, int sock, char *message) {
  size_t s_buff = strlen(message) + 51;
  char buff[s_buff];
  sprintf(buff, "[client %d] %s", sock, message);
  writeLog(level, buff);
}
