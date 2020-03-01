#include "context.h"
#include "sds/sds.h"
#include <stdio.h>
#include <stdbool.h>

#define isOdd(num) (((num)&1)==1)
#define ERROR(...) do {printf(__VA_ARGS__);exit(-1);} while(0)

cnode_t* cnode_from_dict(dict_t* dict) {
  cnode_t* node = malloc(sizeof(*node));
  node->type = node_DICT;
  node->d = dict;
  return node;
}

cnode_t* cnode_from_tokens(l0_listof(sds) slist) {
  cnode_t* node = malloc(sizeof(*node));
  node->type = node_TOKENS;
  node->t = slist;
  return node;
}

cnode_t* cnode_from_list(l0_listof(cnode_t*) list) {
  cnode_t* node = malloc(sizeof(*node));
  node->type = node_LIST;
  node->l = list;
  return node;
}  

static void ctxt_show_imp(dict_t* yaml, int indent);

static void print_escaped(const char* s) {
  bool needs_quotes = strchr(s,' ');
  if (needs_quotes) {  printf("\""); }
  printf("%s",s);
  if (needs_quotes) {  printf("\""); }
}

void cnode_show(cnode_t* cnode, int indent) {
  switch (cnode->type) {
  case node_NIL:
    printf(" ");
    break;
  case node_STR:
    printf("%s", cnode->s);
    break;
  case node_INT:
    printf("%ld", cnode->i);
    break;
  case node_FLOAT:
    printf("%f", cnode->f);
    break;
  case node_TOKENS:
    {
      l0_listof(sds) it = cnode->t;
      for (;it;it = l0_next(it)) {
        print_escaped(*it);
        printf(" ");
      }
    }
    break;
  case node_DICT:
    printf("\n");
    ctxt_show_imp(cnode->d, indent+2);
    break;
  case node_LIST:
    {
      l0_listof(cnode_t*) it = cnode->l;
      printf("[");
      for (;it;it = l0_next(it)) {
        cnode_show(*it, indent+2);
        printf("\n");
      }
      printf("%*s]",indent,"");
      break;
    }
  }
}

int ctxt_each(dict_t* node, ctxt_iterator_cb cb, void* userdata) {
  khiter_t k;
  for (k = kh_first(node); k!=kh_end(node); k=kh_next(node,k)) {
    const char* key = kh_key(node,k);
    cnode_t* val = (cnode_t*)kh_value(node,k);
    int r = cb(userdata,key,val);
    if (r<0) return r;
    if (r>0) continue;
  }
  return 0;
}

static void ctxt_show_imp(dict_t* yaml, int indent) {
  dict_t* node = yaml;
  
  khiter_t k;
  for (k = kh_first(node); k!=kh_end(node); k=kh_next(node,k)) {
    const char* key = kh_key(node,k);
    cnode_t* cnode = (cnode_t*)kh_value(node,k);
    printf("%*s", indent, ""); //indent spaces
    print_escaped(key);
    printf(" ");
    cnode_show(cnode, indent);
    printf("\n");
  }
}

void ctxt_show(dict_t* yaml) {
  ctxt_show_imp(yaml,0);
}


cnode_t* ctxt_node_from(const char* str)
{
  cnode_t* retval = malloc(sizeof(*retval));
  if (strchr(str,'.')) { //might be a float
    char* end;
    double d = strtod(str,&end);
    if (*str && !*end) { //good conversion
      retval->type = node_FLOAT;
      retval->f = d;
      return retval;
    }
  }
  else {//might be an int
    char* end;
    int64_t l = strtol(str,&end,0);
    if (*str && !*end) { //good conversion
      retval->type = node_INT;
      retval->i = l;
      return retval;
    }
  }
  //else we are a string
  retval->type = node_STR;
  retval->s = strdup(str);
  return retval;
}


dict_t* ctxt_store(FILE* f, sds linebuf, int baseindent) {
  char* next_line;
  
  dict_t *current_hash = dict_new();
  cnode_t* listnode = NULL;
  
  int repeat = 0;
  sds this_line = sdsempty();
  while (repeat||(next_line = fgets(linebuf, 256, f))) {
    this_line = sdscpy(this_line, next_line);
    //--or sdsclear(this_line);sdscat(this_line,next_line);
    
    sdstrimtail(this_line, " \n");   //trim trailing newline
    if (strchr(this_line, '\t')) {
      ERROR("Tabs not allowed");
    }
    int thislinelen=sdslen(this_line);
    repeat=0;

    // compute the indentation level
    int p;
    for (p = 0; p < thislinelen && this_line[p]==' '; p++) {
      ;
    }
    int indent = p;

    // check if comment or empty
    if (p == thislinelen || this_line[p] == '#' ) {
      continue;
    }
    if (indent < baseindent) {
      break;
    }
    //Split into quoted parts
    int i,count = 0;
    sds* tokens = sdssplitlen(this_line+p, thislinelen-p, "\"", 1, &count);
    //An even number of quotes results in an odd number of tokens.
    if (!isOdd(count)) {
      ERROR("missing \" in config line\n %s\n",this_line);
    }
    l0_listof(sds) tokenlist = NULL;

    //If the first token has zero length, then the whole string started with a quote.
    bool quoted = false;
    
    for (i=0;i<count;i++) {
      if (sdslen(tokens[i]) > 0) { //skip empty strings
        //Add quoted strings as-is
        if (quoted) {
          l0_add(tokenlist, sdsdup(tokens[i]));
        }
        else {
          //split on space
          int j,count2 = 0;
          sds* tokens2 = sdssplitlen(tokens[i], sdslen(tokens[i]), " ", 1, &count2);
          //add each space-delimited token.
          for (j=0;j<count2;j++) {
            if (sdslen(tokens2[j])) {
              l0_add(tokenlist, sdsdup(tokens2[j]));
            }
          }
          sdsfreesplitres(tokens2,count2);
        }
      }
      quoted = !quoted;
    }
    sdsfreesplitres(tokens,count);
    
    sds key;
    size_t nitems;
    l0_size(tokenlist, &nitems);
    printf("Line <%s> is split into %ld tokens\n", this_line+p, nitems);
    if (nitems) {
      key = l0_peek(tokenlist);      
      l0_pop(tokenlist);
      l0_listof(sds) iter = NULL;
      
      if (1) {
        sds valstr = sdsnewlen("",0);
        for (iter=tokenlist;iter;iter=l0_next(iter)) {
          valstr = sdscat(valstr, *iter);
          valstr = sdscat(valstr," ");
        }
        printf("line length: %u", thislinelen);
        printf(",\tindents: %i", indent);
        printf(",\tkey: %s", key);
        printf(",\tvalue: '%s'\n", valstr);

        sdsfree(valstr);
      }
    }    
    cnode_t *hashval = NULL;
    
    l0_size(tokenlist, &nitems); //how many items after the key:
    if (nitems>0) {
      if (!strcmp(*tokenlist, "[")) { //making a list
        dict_t* nested = ctxt_store(f, linebuf, indent+1);
        if (!listnode) {
          listnode = cnode_from_list(NULL);          
          hashval = listnode;
        }
        l0_add(listnode->l, cnode_from_dict(nested));
        printf("storing {%s:LIST}\n", key);
        repeat=!feof(f);
      }
      else {
        hashval = cnode_from_tokens(tokenlist);
        printf("storing {%s:%s}\n", key,*tokenlist);
      }
    }
    else {
      if (!strcmp(key, "]")) { //ending list entry, do't create new node
        listnode = NULL;
      }
      else {
        dict_t* nested = ctxt_store(f, linebuf, indent+1);
        hashval = cnode_from_dict(nested);
        printf("^created {%s:NEWNODE}\n", key);
        repeat=!feof(f);
      }
    }
    if (hashval) {
      dict_set(current_hash, key, hashval);
    }
  }
  sdsfree(this_line);
  return current_hash;
}
dict_t * ctxt_read(const char *file) {
  printf("reading file %s\n", file);
  sds linebuf = sdsnewlen("",256);
  FILE *f = fopen(file, "r");
  

  if (f == NULL) {
    perror(file);
  }

  dict_t* parent_hash = ctxt_store(f,linebuf,0);

  fclose(f);
  sdsfree(linebuf);
  return parent_hash;
}



void cnode_destroy(cnode_t* node)
{
  switch (node->type) {
  case node_NIL:
    break;
  case node_STR:
    break;
  case node_INT:
    break;
  case node_FLOAT:
    break;
  case node_TOKENS:
    {
      l0_listof(sds) list = node->t;
      while (list) {
        sdsfree(*list);
        l0_pop(list);
      }
    }
    break;
  case node_DICT:
    ctxt_destroy(node->d);
    break;
  case node_LIST:
    {
      l0_listof(cnode_t*) list = node->l;
      while (list) {
        cnode_destroy(*list);
        l0_pop(list);
      }
    }
    break;
  }
  free(node);
}


static void destroy_cb(dict_t* self, const char* key, void* val) {
  printf("deleting %s:\n", key);
  dict_del(self, key);
  cnode_destroy((cnode_t*)val);
  sdsfree((char*)key);
}

void ctxt_destroy(dict_t* self) {
  dict_each(self, {
      destroy_cb(self, key,val);
      
    });

  dict_clear(self);
  dict_free(self);
  
}

