#include <stdbool.h>
#include <stdint.h>

#ifndef UTILS_H
#define UTILS_H

char *trimOnlySpaceAndTab(char *str);
bool str_to_uint16(const char *str, uint16_t *res);
bool strempty(char *str);
char *strcpyft(char *str, int from, int to);
void strkeyval(char *str, char *separator, char **key, char **value);
void strrm(char *str, char character);

#endif
