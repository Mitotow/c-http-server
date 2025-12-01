#include "strutils.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
bool strempty(char *str) { return (str == NULL || strcmp(str, "") != 0); }
