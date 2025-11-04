#include "fm.h"
#include "http.h"
#include "logger.h"
#include "router.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long getFileSize(FILE *file, char *path) {
  bool isFile = (file != NULL);
  if (!isFile) {
    file = fopen(path, "rb");
    if (file == NULL) {
      return -1;
    }
  }

  writeLog(LOG_DEBUG, "Retrieving file size");

  // Retrieve file size
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  writeLog(LOG_DEBUG, "File size : %ld", size);

  if (!isFile)
    fclose(file);

  return size;
}

// Retrieve file path from a string
char *getFilePath(char *str) {
  char *path;
  size_t size = strlen(PUBLIC_DIR) + 1 + strlen(str) + 1;
  path = (char *)malloc(size);

  strcpy(path, PUBLIC_DIR);
  strcat(path, str);

  return path;
}

// Retrieve file path from a request
char *getFilePathFromRequest(request_t req) { return getFilePath(req.route); }

// Retrieve file path from a route
char *getFilePathFromRoute(route_t route) {
  return getFilePath(route.fileName);
}

// Read file, return it's content and set the s
// parameter as the size of the file
char *readFile(const char *ctype, const char *path, long *s) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    return NULL;
  }

  *s = getFileSize(file, (char *)path);

  if (*s == -1) {
    writeLog(LOG_DEBUG, "Path does not exists");
    fclose(file);
    return NULL;
  }

  // Alloc buffer for the content
  char *buffer = (char *)malloc(*s);
  if (!buffer) {
    writeLog(LOG_ERROR, "Cannot allocate memory to read file");
    fclose(file);
    return NULL;
  }

  // Read file content
  size_t bytes_read = fread(buffer, 1, *s, file);
  if (bytes_read != (size_t)*s) {
    writeLog(LOG_ERROR, "Error while reading a file");
    free(buffer);
    fclose(file);
    return NULL;
  }

  if (strcmp(ctype, CTYPE_HTML) == 0) {
    char *txt_buffer = (char *)realloc(buffer, *s + 1);
    if (!txt_buffer) {
      writeLog(LOG_ERROR, "Problem when trying to realloc buffer");
      free(buffer);
      fclose(file);
      return NULL;
    }
    buffer = txt_buffer;
    buffer[*s] = '\0';
  }

  fclose(file);
  return buffer;
}

// Retrieve the extension of the file of a route, return NULL if the route
// does not point to a specific file
char *getExtension(const char *route) {
  char *pt = strrchr(route, '.');
  if (pt == NULL) {
    return NULL;
  } else {
    return pt + 1;
  }
}
