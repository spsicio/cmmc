#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"

Type type_int = (Type) {
    .kind = kINT_t, };

Type type_flt = (Type) {
    .kind = kFLT_t, };

Type* get_type_int() { return &type_int; }
Type* get_type_flt() { return &type_flt; }

void free_field(Field *p) {
  if (p == NULL) return;
  free_field(p->nxt);
  free_type(p->type);
  free(p);
}

void free_type(Type *p) {
  if (p == NULL) return;
  if (p->kind == kARRAY) {
    free_type(p->array.elem_t);
  } else if (p->kind == kSTRCT) {
    free_field(p->strct.fields);
  } else if (p->kind == kFUNCT) {
    free_type(p->funct.ret_t);
    free_field(p->funct.params);
  }
  free(p);
}

bool type_eq(Type *p, Type *q) {
  if (p->kind != q->kind) return false;
  switch (p->kind) {
    case kINT_t:
    case kFLT_t: return true;
    case kARRAY:
      if (p->array.dim != q->array.dim) return false;
      for (int i=1; i<p->array.dim; ++i) {
        if (p->array.len[i] != q->array.len[i]) return false;
      }
      return type_eq(p->array.elem_t, q->array.elem_t);
    case kSTRCT:
      return !strcmp(p->strct.name, q->strct.name);
  }
  fprintf(stderr, "REACHING INVALID AREA in type_eq.\n");
}

bool get_field(Field *p, const char* name) {
  while (p != NULL) {
    if (!strcmp(p->name, name)) return true;
    p = p->nxt;
  }
  return false;
}

