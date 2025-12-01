#include <stdbool.h>
#include <stdint.h>

#ifndef UTILS_H
#define UTILS_H

char *trimOnlySpaceAndTab(char *str);
bool str_to_uint16(const char *str, uint16_t *res);
bool strempty(char *str);

#endif
