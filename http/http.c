#include "http.h"
#include <string.h>

// Get a status by its code
const status_t *getStatus(int code) {
  for (size_t i = 0; i < sizeof(status) / sizeof(status[0]); i++) {
    if (status[i].code == code) {
      return &status[i];
    }
  }

  return NULL;
}

// Return the corresponding content type
const char *getContentType(char *ext) {
  if (ext == NULL) {
    return CTYPE_PLAIN;
  }

  for (int i = 0; i < sizeof(content_types) / sizeof(content_types[0]); i++) {
    if (strcmp(content_types[i].ext, ext) == 0) {
      return content_types[i].type;
    }
  }

  return CTYPE_PLAIN;
}

// Check if content type is text
bool isTextContentType(char *ctype) {
  if (strlen(ctype) < 5) {
    return false;
  }

  for (int i = 0; i < 5; i++) {
    if (ctype[i] != CTYPE_TEXT_PREFIX[i]) {
      return false;
    }
  }

  return true;
}
