#ifndef CACHE_H
#define CACHE_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

typedef struct cache_entry {
  char *filepath;
  char *content;
  size_t size;
  time_t last_access;
  time_t created_at;
  struct cache_entry *prev;
  struct cache_entry *next;
  struct cache_entry *hash_next;
} cache_entry_t;

typedef struct {
  cache_entry_t **hash_table;
  cache_entry_t *head;
  cache_entry_t *tail;
  size_t max_entries;
  size_t current_entries;
  time_t ttl;
  pthread_mutex_t mutex;
} file_cache_t;

file_cache_t *create_file_cache(size_t max_entries, time_t ttl);
cache_entry_t *get_cache_entry_by(file_cache_t *cache,
                                  bool (*filter)(cache_entry_t *entry));
cache_entry_t *get_cache_entry_by_path(file_cache_t *cache, char *filepath);
cache_entry_t *get_cache_entry_access(file_cache_t *cache);
void add_cache_entry(file_cache_t *cache, char *filepath, size_t size,
                     char *content);
void rm_cache_entry(file_cache_t *cache, cache_entry_t *entry);
void rm_cache_entry_by_path(file_cache_t *cache, char *filepath);
void clean_cache(file_cache_t *cache);
bool is_full(file_cache_t *cache);
bool is_empty(file_cache_t *cache);
void cache_print_debug(file_cache_t *cache);
void destroy_cache(file_cache_t *cache);

#endif
