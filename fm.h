#ifndef FM_H
#define FM_H
#include "request.h"
#include "router.h"

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
#define EXT_JPEG_ALT "jpeg"
#define EXT_PNG "png"
#define EXT_TIFF "tif"
#define EXT_WOFF "woff"
#define EXT_WOFF2 "woff2"
#define EXT_TTF "ttf"
#define EXT_OTF "otf"
#define EXT_EOT "eot"

char *getFilePathFromRequest(request_t req);
char *getFilePathFromRoute(route_t route);
char *readFile(const char *ctype, const char *path, long *s);
char *getExtension(const char *path);

#endif
