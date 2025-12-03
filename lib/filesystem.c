#include "filesystem.h"
#include "../utils/strutils.h"
#include "logger.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// Get the file content size
long getFileSize(FILE *file) {
  if (file == NULL)
    return -1;

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  return size;
}

// Get the file content size from a file path
long getFileSizeFromPath(char *path) {
  FILE *file = fopen(path, "rb");
  if (!file)
    return -1;

  long size = getFileSize(file);
  fclose(file);
  return size;
}

// Read file and return its content
char *readFile(char *path, long *size) {
  FILE *file = fopen(path, "rb"); // Open file in read byte mode
  if (!file) {
    return NULL;
  }

  char *buffer = NULL;
  size_t bytes_read = -1;
  long fileSize = getFileSize(file);

  if (fileSize >= 0) {
    buffer = (char *)malloc(sizeof(char) * fileSize);
    bytes_read = fread(buffer, 1, fileSize, file);

    if (bytes_read != fileSize) {
      free(buffer);
      buffer = NULL;
    }
  }

  fclose(file);
  *size = fileSize;
  return buffer;
}

// Read file, return its content and add the EOL char
char *readTextFile(char *path, long *size) {
  char *buffer = readFile(path, size);
  if (buffer != NULL) {
    buffer = realloc(buffer, sizeof(char) * *size + 1);
    buffer[*size] = '\0';
  }
  return buffer;
}

// Retrieve the extension of a path to a file
char *getExtension(char *path) {
  char *pt = strrchr(path, '.');
  if (pt == NULL) {
    return NULL;
  } else {
    return pt + 1;
  }
}

// Retrieve file path from a string
char *getFilePath(server_context_t *ctx, char *str) {
  char *path;
  size_t size = ctx->config->base_href_length + 1 + strlen(str) + 1;
  path = (char *)malloc(size);

  strcpy(path, ctx->config->base_href);
  strcat(path, str);

  return path;
}

char *getAbsolutePath(char *path) {
  char *resolved_path = malloc(sizeof(char) * PATH_MAX);
  char *strcp = strdup(path);
  strrm(strcp, '\n');
  if (realpath(strcp, resolved_path) == NULL) {
    free(resolved_path);
    return NULL;
  }

  free(strcp);
  return resolved_path;
}

// Check if path exists and result is inside context base href
bool is_valid_path(server_context_t *ctx, char *path) {
  // Will always point to base_href
  if (path == NULL)
    return true;

  char *ptr;
  ptr = getAbsolutePath(path);
  if (ptr == NULL) {
    return false;
  }

  // Check if path starts with base href
  bool is_valid = strlen(ptr) > ctx->config->base_href_length;
  if (is_valid) {
    for (uint16_t i = 0; i < ctx->config->base_href_length; i++) {
      if (ptr[i] != ctx->config->base_href[i]) {
        is_valid = false;
        break;
      }
    }
  }

  free(ptr);
  return is_valid;
}
