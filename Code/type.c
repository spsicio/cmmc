#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"

Type type_int = (Type) {
    .kind = kINT_t, };

Type type_flt = (Type) {
    .kind = kFLT_t, };

Type type_err = (Type) {
    .kind = kERR_t, };

Field* alloc_field(Type *type, const char *str) {
  Field *p = malloc(sizeof(Field));
  strcpy(p->name, str);
  p->type = type;
  p->nxt = NULL;
  return p;
}

void free_field(Field *p) {
  if (p == NULL) return;
  free_field(p->nxt);
  free(p);
}

bool type_eq(Type *p, Type *q) {
  if (p->kind == kERR_t || q->kind == kERR_t) return true;
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

bool type_isprim(Type *type) {
  if (type == NULL) return false;
  if (type->kind == kINT_t) return true;
  if (type->kind == kFLT_t) return true;
  if (type->kind == kERR_t) return true;
  return false;
}

bool type_isdisc(Type *type) {
  if (type == NULL) return false;
  if (type->kind == kINT_t) return true;
  if (type->kind == kERR_t) return true;
  return false;
}

Field* get_field(Field *p, const char* name) {
  while (p != NULL) {
    if (!strcmp(p->name, name)) return p;
    p = p->nxt;
  }
  return NULL;
}

