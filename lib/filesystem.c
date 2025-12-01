#include "filesystem.h"
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

  if (file != NULL) {
    fclose(file);
  }

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
char *getFilePath(char *str) {
  char *path;
  size_t size = strlen(PUBLIC_DIR) + 1 + strlen(str) + 1;
  path = (char *)malloc(size);

  strcpy(path, PUBLIC_DIR);
  strcat(path, str);

  printf("path : %s\n", path);
  return path;
}
