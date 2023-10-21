#include <stdlib.h>
#include <string.h>
#include "symtab.h"

Symtab *env = NULL;

void scope_push() {
  Symtab *p = (Symtab*) malloc(sizeof(Symtab));
  p->root = NULL;
  p->nxt = env;
  env = p;
}

void scope_pop() {
  if (env == NULL) return;
  Symtab *p = env->nxt;
  free_avl(env->root);
  free(env);
  env = p;
}

Syment* symtab_lookup(const char *name) {
  for (Symtab *p = env; p != NULL; p = p->nxt) {
    AVLNode *res = avl_find(p->root, name);
    if (res != NULL) return res->ent;
  }
  return NULL;
}

Syment* symtab_insert(const char *name) {
  AVLNode *res = avl_find(env->root, name);
  if (res != NULL) return NULL;
  Syment *ent = (Syment*) malloc(sizeof(Syment));
  strcpy(ent->name, name);
  env->root = avl_insert(env->root, name, ent);
  return ent;
}

