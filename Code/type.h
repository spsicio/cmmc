#ifndef CMMC_TYPE_H
#define CMMC_TYPE_H

#include <stdint.h>
#include <stdbool.h>
#include "lexer.h"

#define MAX_ARRAY_DIMSN_NUM (32)
#define MAX_STRCT_FIELD_NUM (32)
#define MAX_FUNCT_PARAM_NUM (32)

typedef enum {
  kINT_t,
  kFLT_t,
  kARRAY,
  kSTRCT,
  kFUNCT,
  kERR_t,
} TYP_KIND;

typedef struct Field {
  symstr name;
  struct Type* type;
  struct Field* nxt;
} Field, Param;

typedef struct Type {
  TYP_KIND kind;
  union {
    struct {
      struct Type *elem_t;
      uint32_t len[MAX_ARRAY_DIMSN_NUM], dim;
    } array;
    struct {
      symstr name;
      Field *fields;
    } strct;
    struct {
      struct Type *ret_t; 
      Param *params;
      int paramc;
    } funct;
  };
} Type;

extern Type type_int;
extern Type type_flt;
extern Type type_err;
Field* alloc_field(Type*, const char*);
void free_field(Field*);
bool type_eq(Type*, Type*);
bool type_isprim(Type*);
bool type_isdisc(Type*);
Field* get_field(Field*, const char*); 

#endif  // CMMC_TYPE_H

