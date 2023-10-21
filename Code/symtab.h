#ifndef CMMC_SYMTAB_H
#define CMMC_SYMTAB_H

#include "avl.h"
#include "type.h"

typedef enum {
  kTYP,
  kVAR,
  kFUN,
} SYM_KIND;

typedef struct Symtab {
  AVLNode *root;
  struct Symtab *nxt;
} Symtab;

typedef struct Syment {
  SYM_KIND kind;
  char name[MAX_TOKEN_LEN];
  Type *type;
} Syment;

void scope_push();
void scope_pop();
Syment* symtab_lookup(const char*);
Syment* symtab_insert(const char*);

#endif  // CMMC_SYMTAB_H

