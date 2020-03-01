#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "list0/l0.h"
#include "sds/sds.h"
#include "dict.h"

typedef struct context_node {
  enum {
        node_NIL,
        node_STR,
        node_INT,
        node_FLOAT,
        node_DICT,
        node_TOKENS,
        node_LIST,
  } type;
  union {
    const char* s;
    int64_t i;
    double f;
    dict_t* d;
    l0_listof(sds) t; //tokenlist
    l0_listof(struct context_node*) l; //nodelist
    
  };
} cnode_t;


dict_t * ctxt_read(const char *file);
void ctxt_destroy(dict_t* self);
void ctxt_show(dict_t* data);

cnode_t* cnode_from_dict(dict_t* data);


static inline dict_t* cnode_as_dict(cnode_t* cnode) {
  return (cnode->type==node_DICT)?(cnode->d):NULL;
}

typedef int (*ctxt_iterator_cb)(void* userdata, const char* key, cnode_t* val);
int ctxt_each(dict_t* data, ctxt_iterator_cb cb, void* userdata);

#endif
