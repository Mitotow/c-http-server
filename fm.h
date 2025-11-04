#include "request.h"
#include "router.h"
#include <stdio.h>

#ifndef FM_H
#define FM_H

// Path
#define BASE_DIR "."
#define PUBLIC_DIR "./public/"

// Extensions
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
#define EXT_SVG "svg"
#define EXT_WOFF "woff"
#define EXT_WOFF2 "woff2"
#define EXT_TTF "ttf"
#define EXT_OTF "otf"
#define EXT_EOT "eot"

long getFileSize(FILE *file, char *path);
char *getFilePathFromRequest(request_t req);
char *getFilePathFromRoute(route_t route);
char *readFile(const char *ctype, const char *path, long *s);
char *getExtension(const char *path);

#endif
