
//
// dict.c - a generic dictionary
//
// derived from hash.c
// Copyright (c) 2012 TJ Holowaychuk <tj@vision-media.ca>
//

#include "dict.h"

/*
 * Set dict `key` to `val`.
 */

inline void
dict_set(dict_t *self, char *key, void *val) {
  int ret;
  khiter_t k = kh_put(ptr, self, key, &ret);
  kh_value(self, k) = val;
}

/*
 * Get dict `key`, or NULL.
 */

inline void *
dict_get(dict_t *self, const char *key) {
  khiter_t k = kh_get(ptr, self, key);
  return k == kh_end(self) ? NULL : kh_value(self, k);
}

/*
 * Check if dict `key` exists.
 */

inline int
dict_has(dict_t *self, char *key) {
  khiter_t k = kh_get(ptr, self, key);
  return kh_exist(self, k);
}

/*
 * Remove dict `key`.
 */

void
dict_del(dict_t *self, const char *key) {
  khiter_t k = kh_get(ptr, self, key);
  kh_del(ptr, self, k);
}

// tests

#ifdef TEST_DICT

#include <stdio.h>
#include <assert.h>
#include <string.h>

void
test_dict_set() {
  dict_t *dict = dict_new();
  assert(0 == dict_size(dict));

  dict_set(dict, "name", "tobi");
  dict_set(dict, "species", "ferret");
  assert(2 == dict_size(dict));

  assert(0 == strcmp("tobi", dict_get(dict, "name")));
  assert(0 == strcmp("ferret", dict_get(dict, "species")));
}

void
test_dict_get() {
  dict_t *dict = dict_new();
  dict_set(dict, "foo", "bar");
  assert(0 == strcmp("bar", dict_get(dict, "foo")));
  assert(NULL == dict_get(dict, "bar"));
}

void
test_dict_has() {
  dict_t *dict = dict_new();
  dict_set(dict, "foo", "bar");
  assert(1 == dict_has(dict, "foo"));
  assert(0 == dict_has(dict, "bar"));
}

void
test_dict_size() {
  dict_t *dict = dict_new();
  assert(0 == dict_size(dict));
  dict_set(dict, "foo", "bar");
  assert(1 == dict_size(dict));
  dict_set(dict, "bar", "baz");
  assert(2 == dict_size(dict));
}

void
test_dict_del() {
  dict_t *dict = dict_new();
  dict_set(dict, "foo", "bar");
  assert(1 == dict_has(dict, "foo"));
  assert(0 == dict_has(dict, "bar"));
  dict_del(dict, "foo");
  dict_del(dict, "bar");
  assert(0 == dict_has(dict, "foo"));
}

void
test_dict_clear() {
  dict_t *dict = dict_new();
  dict_set(dict, "foo", "bar");
  dict_set(dict, "bar", "baz");
  dict_set(dict, "raz", "jaz");
  assert(3 == dict_size(dict));
  dict_clear(dict);
  assert(0 == dict_size(dict));
}

void
test_dict_each() {
  dict_t *dict = dict_new();
  dict_set(dict, "name", "tj");
  dict_set(dict, "age", "25");

  const char *keys[2];
  void *vals[2];
  int n = 0;

  dict_each(dict, {
    keys[n] = key;
    vals[n] = val;
    n++;
  });

  assert(0 == strcmp("age", keys[0]) || 0 == strcmp("name", keys[0]));
  assert(0 == strcmp("age", keys[1]) || 0 == strcmp("name", keys[1]));
  assert(0 == strcmp("25", vals[0]) || 0 == strcmp("tj", vals[0]));
  assert(0 == strcmp("25", vals[1]) || 0 == strcmp("tj", vals[1]));
}

void
test_dict_each_key() {
  dict_t *dict = dict_new();
  dict_set(dict, "name", "tj");
  dict_set(dict, "age", "25");

  const char *keys[2];
  int n = 0;

  dict_each_key(dict, {
    keys[n++] = key;
  });

  assert(0 == strcmp("age", keys[0]) || 0 == strcmp("name", keys[0]));
  assert(0 == strcmp("age", keys[1]) || 0 == strcmp("name", keys[1]));
}

void
test_dict_each_val() {
  dict_t *dict = dict_new();
  dict_set(dict, "name", "tj");
  dict_set(dict, "age", "25");

  void *vals[2];
  int n = 0;

  dict_each_val(dict, {
    vals[n++] = val;
  });

  assert(0 == strcmp("25", vals[0]) || 0 == strcmp("tj", vals[0]));
  assert(0 == strcmp("25", vals[1]) || 0 == strcmp("tj", vals[1]));
}

int
main(){
  test_dict_set();
  test_dict_get();
  test_dict_has();
  test_dict_del();
  test_dict_size();
  test_dict_clear();
  test_dict_each();
  test_dict_each_key();
  test_dict_each_val();
  printf("\n  \e[32m\u2713 \e[90mok\e[0m\n\n");
  return 0;
}

#endif
