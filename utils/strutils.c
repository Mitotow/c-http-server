#include "strutils.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

bool startsWithSpaceOrTab(char *str) {
  if (strlen(str) < 1) {
    return false;
  }

  return (str[0] == ' ' || str[0] == '\t');
}

char *trimOnlySpaceAndTab(char *str) {
  char *end;

  // Trim leading space
  while (startsWithSpaceOrTab(str))
    str++;

  if (*str == 0) // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while (end > str && startsWithSpaceOrTab(end))
    end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

// Remove the \n char at the end of a string
char *strchomp(char *str) {
  if (!str)
    return NULL;

  size_t len = strlen(str);

  if (len > 0 && str[len - 1] == '\n') {
    str[len - 1] = '\0';
  }

  return str;
}

bool str_to_uint16(const char *str, uint16_t *res) {
  char *endptr;

  errno = 0;
  long result = strtoul(str, &endptr, 10);

  // Vérifier les erreurs
  if (endptr == str || errno != 0 || result > UINT16_MAX) {
    return false;
  }
  *res = (uint16_t)result;
  return true;
}

// Check if a string is empty
bool strempty(char *str) { return (str == NULL || strcmp(str, "") == 0); }

// Slice a string
char *strcpyft(char *str, int from, int to) {
  int strIndex = 0;
  ulong size = to - from;
  char *res = malloc(sizeof(char) * size + 1);
  for (int i = from; i <= to; i++) {
    res[strIndex] = str[i];
    strIndex++;
  }
  res[size] = '\0';

  return res;
}

void strkeyval(char *str, char *separator, char **key, char **value) {
  char *dup = strdup(str);
  char *saveptr = NULL;

  *key = strtok_r(dup, separator, &saveptr);
  if (*key != NULL) {
    *key = trimOnlySpaceAndTab(*key);
    *key = strdup(*key);
  }

  *value = strtok_r(NULL, separator, &saveptr);
  if (*value != NULL) {
    *value = trimOnlySpaceAndTab(*value);
    *value = strdup(*value);
  }

  free(dup);
}

void strrm(char *str, char character) {
  if (!str)
    return;

  char *src = str, *dst = str;

  while (*src) {
    if (*src != character) {
      *dst++ = *src;
    }
    src++;
  }
  *dst = '\0';
}
