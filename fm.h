#include "request.h"
#include "router.h"
#ifndef FM_H
#define FM_H

// File
#define BASE_DIR "."
#define PUBLIC_DIR "./public/"
#define EXT_HTML "html"
#define EXT_JS "js"
#define EXT_CSS "css"
#define EXT_CSV "csv"
#define EXT_XML "xml"
#define EXT_ICO "ico"
#define EXT_GIF "gif"
#define EXT_JPEG "jpg"
#define EXT_PNG "png"
#define EXT_TIFF "tif"

char *getFilePathFromRequest(request_t req);
char *getFilePathFromRoute(route_t route);
char *readFile(const char *ctype, const char *path, long *s);
char *getExtension(const char *path);

#endif
