#include "request.h"
#ifndef FM_H
#define FM_H

// File
#define BASE_DIR "."
#define PUBLIC_DIR "./public"
#define EXT_ICO "ico"
#define EXT_HTML "html"

char *getFilePathFromRequest(struct request req);
char *readFile(const char *ctype, const char *path, long *s);
char *getExtension(const char *path);

#endif
