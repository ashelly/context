
//
// dict.h - a generic dictionary
//
// derived from hash.h
// Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
//

#ifndef DICT_H_
#define DICT_H_

#include "khash.h"

// pointer dict

KHASH_MAP_INIT_STR(ptr, void *);

/*
 * Dict type.
 */

typedef khash_t(ptr) dict_t;

/*
 * Allocate a new dict.
 */

#define dict_new() kh_init(ptr)

/*
 * Destroy the dict.
 */

#define dict_free(self) kh_destroy(ptr, self)

/*
 * Dict size.
 */

#define dict_size kh_size

/*
 * Remove all pairs in the dict.
 */

#define dict_clear(self) kh_clear(ptr, self)

/*
 * Iterate dict keys and ptrs, populating
 * `key` and `val`.
 */

#define dict_each(self, block) { \
   const char *key; \
   void *val; \
    for (khiter_t k = kh_begin(self); k < kh_end(self); ++k) { \
      if (!kh_exist(self, k)) continue; \
      key = kh_key(self, k); \
      val = kh_value(self, k); \
      block; \
    } \
  }

/*
 * Iterate dict keys, populating `key`.
 */

#define dict_each_key(self, block) { \
    const char *key; \
    for (khiter_t k = kh_begin(self); k < kh_end(self); ++k) { \
      if (!kh_exist(self, k)) continue; \
      key = kh_key(self, k); \
      block; \
    } \
  }

/* #define dict_for_each(self)\ */
/*     const char *key; \ */
/*     void* val; \ */
/*     khiter_t k; */
/* for (k = kh_first(self,k); k < kh_end(self) &&  (key = kh_key(self, k) && ((val=kh_val(self,k))||1)); k=kh_next(self,k)) */
  
/*
 * Iterate dict ptrs, populating `val`.
 */

#define dict_each_val(self, block) { \
    void *val; \
    for (khiter_t k = kh_begin(self); k < kh_end(self); ++k) { \
      if (!kh_exist(self, k)) continue; \
      val = kh_value(self, k); \
      block; \
    } \
  }

// protos

void
dict_set(dict_t *self, char *key, void *val);

void *
dict_get(dict_t *self, const char *key);

int
dict_has(dict_t *self, char *key);

void
dict_del(dict_t *self, const char *key);

void
dict_clear(dict_t *self);


static inline khiter_t kh_first(dict_t* h) {
  khiter_t k=kh_begin(h);
  while (k !=kh_end(h) && !kh_exist(h,k)) {
    ++k;
  }
  return k;
}

static inline khiter_t kh_next(dict_t* h, khiter_t k) {
  do {
    if (k==kh_end(h)) break;
    ++k;
  } while (!kh_exist(h,k));
  return k;
}


#endif /* DICT_H_ */
