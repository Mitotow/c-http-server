#include "http.h"
#include "fm.h"
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
char *getContentType(char *ext) {
  if (ext == NULL) {
    return CTYPE_PLAIN;
  }

  if (strcmp(ext, EXT_HTML) == 0) {
    return CTYPE_HTML;
  } else if (strcmp(ext, EXT_JS) == 0) {
    return CTYPE_JS;
  } else if (strcmp(ext, EXT_CSS) == 0) {
    return CTYPE_CSS;
  } else if (strcmp(ext, EXT_CSV) == 0) {
    return CTYPE_CSV;
  } else if (strcmp(ext, EXT_XML) == 0) {
    return CTYPE_XML;
  } else if (strcmp(ext, EXT_ICO) == 0) {
    return CTYPE_ICO;
  } else if (strcmp(ext, EXT_GIF) == 0) {
    return CTYPE_GIF;
  } else if (strcmp(ext, EXT_JPEG) == 0) {
    return CTYPE_JPEG;
  } else if (strcmp(ext, EXT_PNG) == 0) {
    return CTYPE_PNG;
  } else if (strcmp(ext, EXT_TIFF) == 0) {
    return CTYPE_TIFF;
  }

  return CTYPE_PLAIN;
}
