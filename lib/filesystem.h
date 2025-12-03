#include "../server.h"
#include <stdbool.h>
#include <stdio.h>

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

// Path
#define BASE_DIR "."
#define PUBLIC_DIR "./public"

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

long getFileSizeFromPath(char *path);
char *readFile(char *path, long *size);
char *readTextFile(char *path, long *size);
char *getExtension(char *path);
char *getFilePath(server_context_t *ctx, char *path);
char *getAbsolutePath(char *path);
bool is_valid_path(server_context_t *ctx, char *path);

#endif
