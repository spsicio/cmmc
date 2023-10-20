#ifndef CMMC_HASHTAB_H
#define CMMC_HASHTAB_H

#define MAX_HASHTAB_SIZ (1024)

#include <stdint.h>

typedef struct Hashent {
  const char* str;
  void *ptr;
  struct Hashent *nxt;
} Hashent;

typedef struct {
  Hashent* head[MAX_HASHTAB_SIZ];
} Hashtab;

uint32_t hash_str();
Hashent* hash_find(Hashtab*, const char*, uint32_t); 
Hashent* hash_find_str(Hashtab*, const char *);

#endif  // CMMC_HASHTAB_H

