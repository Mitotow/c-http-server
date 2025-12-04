#include <stdio.h>

#ifndef MEMUTILS_H
#define MEMUTILS_H

#define FREE_ALL(...)                                                          \
  do {                                                                         \
    void *_ptrs[] = {__VA_ARGS__};                                             \
    for (size_t _i = 0; _i < sizeof(_ptrs) / sizeof(_ptrs[0]); _i++) {         \
      if (_ptrs[_i] != NULL)                                                   \
        free(_ptrs[_i]);                                                       \
    }                                                                          \
  } while (0)

#endif
