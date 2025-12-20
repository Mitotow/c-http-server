#include "cache.h"
#include "../utils/memutils.h"
#include "logger.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

// Set all values at 0
void init_file_cache(file_cache_t *fc) {
  fc->hash_table = NULL;
  fc->head = NULL;
  fc->tail = NULL;
  fc->current_entries = 0;
  pthread_mutex_init(&fc->mutex, NULL);
}

file_cache_t *create_file_cache(size_t max_entries, time_t ttl) {
  file_cache_t *fc = (file_cache_t *)malloc(sizeof(file_cache_t));
  if (!fc) {
    writeLog(LOG_ERROR, "Cannot allocate memory for file cache!");
    return NULL;
  }

  init_file_cache(fc);
  fc->max_entries = max_entries;
  fc->ttl = ttl;

  return fc;
}

void update_last_access(cache_entry_t *entry) {
  if (entry) {
    entry->last_access = time(NULL);
  }
}

cache_entry_t *get_cache_entry_by(file_cache_t *cache,
                                  bool (*filter)(cache_entry_t *entry)) {
  if (is_empty(cache))
    return NULL;
  pthread_mutex_lock(&cache->mutex);
  cache_entry_t *tmp = cache->head;

  while (tmp) {
    if (filter(tmp))
      break;
    tmp = tmp->next;
  }

  update_last_access(tmp);
  pthread_mutex_unlock(&cache->mutex);

  return tmp;
}

cache_entry_t *get_cache_entry_by_path(file_cache_t *cache, char *filepath) {
  if (is_empty(cache))
    return NULL;
  pthread_mutex_lock(&cache->mutex);
  cache_entry_t *tmp = cache->head;

  while (tmp) {
    if (strcmp(filepath, tmp->filepath) == 0)
      break;

    tmp = tmp->next;
  }

  update_last_access(tmp);
  pthread_mutex_unlock(&cache->mutex);

  return tmp;
}

cache_entry_t *get_cache_entry_access(file_cache_t *cache) {
  if (is_empty(cache))
    return NULL;

  pthread_mutex_lock(&cache->mutex);
  cache_entry_t *tmp = cache->head;
  cache_entry_t *ret = cache->head;
  while (tmp) {
    if (tmp->last_access < ret->last_access) {
      ret = tmp;
    }

    tmp = tmp->next;
  }

  update_last_access(ret);
  pthread_mutex_unlock(&cache->mutex);

  return ret;
}

void add_cache_entry(file_cache_t *cache, char *filepath, size_t size,
                     char *content) {
  if (!cache ||
      get_cache_entry_by_path(cache, filepath)) // Check if entry already exists
    return;

  pthread_mutex_lock(&cache->mutex);
  cache_entry_t *entry = (cache_entry_t *)malloc(sizeof(cache_entry_t));
  entry->filepath = filepath;
  entry->size = size;
  entry->content = content;
  entry->next = NULL;
  entry->last_access = time(NULL);
  entry->created_at = time(NULL);

  if (is_empty(cache)) {
    cache->head = entry;
    entry->prev = NULL;
    cache->tail = entry;
  } else if (is_full(cache)) {
    pthread_mutex_unlock(&cache->mutex);
    cache_entry_t *tmp = get_cache_entry_access(cache);
    pthread_mutex_lock(&cache->mutex);
    tmp->prev->next = entry;
    entry->next = tmp->next;
  } else {
    cache->tail->next = entry;
    cache->tail = entry;
  }

  cache->current_entries++;
  pthread_mutex_unlock(&cache->mutex);
}

void rm_cache_entry(file_cache_t *cache, cache_entry_t *entry) {
  if (!cache)
    return;
  pthread_mutex_lock(&cache->mutex);

  entry->prev->next = entry->next;
  entry->next->prev = entry->prev;
  FREE_ALL(entry->content, entry);
  cache->current_entries--;
  pthread_mutex_unlock(&cache->mutex);
}

void rm_cache_entry_by_path(file_cache_t *cache, char *filepath) {
  if (!cache)
    return;
  cache_entry_t *tmp;
  if (!(tmp = get_cache_entry_by_path(cache, filepath)))
    return;

  rm_cache_entry(cache, tmp);
}

void clean_cache(file_cache_t *cache) {
  if (is_empty(cache))
    return;
  pthread_mutex_lock(&cache->mutex);
  cache_entry_t *tmp = cache->head;
  time_t now = time(NULL);

  while (tmp) {
    if (now - tmp->created_at >= cache->ttl) {
      tmp = tmp->next;
      pthread_mutex_unlock(&cache->mutex);
      rm_cache_entry(cache, tmp->prev);
      pthread_mutex_lock(&cache->mutex);
    } else {
      tmp = tmp->next;
    }
  }
  pthread_mutex_unlock(&cache->mutex);
}

bool is_full(file_cache_t *cache) {
  return cache && cache->current_entries >= cache->max_entries;
}

bool is_empty(file_cache_t *cache) { return !cache || !cache->head; }

void cache_print_debug(file_cache_t *cache) {
  writeLog(LOG_DEBUG,
           "Cache - current entries : %d, max entries : %d, ttl : %ld",
           cache->current_entries, cache->max_entries, cache->ttl);
}

void destroy_cache(file_cache_t *cache) {
  pthread_mutex_lock(&cache->mutex);
  if (!is_empty(cache)) {
    cache_entry_t *tmp = cache->head;
    while (tmp) {
      FREE_ALL(tmp->filepath, tmp->content, tmp);

      tmp = tmp->next;
    }
  }

  pthread_mutex_unlock(&cache->mutex);
  pthread_mutex_destroy(&cache->mutex);
  free(cache);
}
