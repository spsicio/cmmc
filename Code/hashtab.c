#include <stddef.h>
#include <string.h>
#include "hashtab.h"

uint32_t hash_str(const char* str) {
  uint32_t res = 0;
  for (const char* p = str; *p; ++p) {
    res = (res * 29 + *p) % MAX_HASHTAB_SIZ;
  }
  return res;
}

Hashent* hash_find(Hashtab *hashtab, const char* str, uint32_t hash_val) {
  Hashent *p = hashtab->head[hash_val]; 
  while (p != NULL) {
    if (!strcmp(str, p->str)) return p;
    p = p->nxt;
  }
  return NULL;
}

Hashent* hash_find_str(Hashtab *hashtab, const char* str) {
  uint32_t hash_val = hash_str(str);
  return hash_find(hashtab, str, hash_val);
}

