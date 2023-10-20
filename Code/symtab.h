#ifndef CMMC_SYMTAB_H
#define CMMC_SYMTAB_H

typedef enum {
  kTYP,
  kVAR,
  kFUN,
} SYM_KIND;

typedef struct Symtab {
  Hashtab hashtab;
  struct Symtab *nxt;
} Symtab;

typedef struct Syment {
  SYM_KIND kind;
} Syment;

#undef  // CMMC_SYMTAB_H

