#include "logger.h"
#include <netinet/in.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <threads.h>
#include <unistd.h>

// Print in stdout a formatted log
void print(char *level, char *format, va_list *args) {
  va_list args_cp;
  va_copy(args_cp, *args);
  int msg_len = vsnprintf(NULL, 0, format, args_cp) + 1;
  va_end(args_cp);

  char prefix[16];
  snprintf(prefix, sizeof(prefix), "[%s] ", level);

  char *buff = malloc(strlen(prefix) + msg_len);
  if (!buff) {
    return;
  }

  vsnprintf(buff, strlen(prefix) + msg_len, format, *args);
  memmove(buff + strlen(prefix), buff, strlen(buff) + 1);
  memcpy(buff, prefix, strlen(prefix));

  if (strcmp(level, LOG_FATAL) == 0 || strcmp(level, LOG_ERROR) == 0) {
    perror(buff);
  } else {
    printf("%s\n", buff);
  }

  free(buff);
}

// Write log in console with a basic format
void writeLog(char *level, char *message, ...) {
  va_list args;
  va_start(args, message);
  print(level, message, &args);
  va_end(args);
}

// Write Fatal log in console with basic format and then exit the process
void writeFatal(char *message, ...) {
  va_list args;
  va_start(args, message);
  print(LOG_FATAL, message, &args);
  va_end(args);

  exit(EXIT_FAILURE);
}
