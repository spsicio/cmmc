#ifndef CMMC_TYPE_H
#define CMMC_TYPE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_ARRAY_DIMSN_NUM (32)
#define MAX_STRCT_FIELD_NUM (32)
#define MAX_FUNCT_PARAM_NUM (32)

typedef struct Field {
  const char* name;
  struct Type* type;
  struct Field* nxt;
} Field, Param;

typedef struct Type {
  enum {
    kINT_t,
    kFLT_t,
    kARRAY,
    kSTRCT,
    kFUNCT,
  } kind;
  union {
    struct {
      struct Type *elem_t;
      uint32_t len[MAX_ARRAY_DIMSN_NUM], dim;
    } array;
    struct {
      const char *name;
      Field *fields;
    } strct;
    struct {
      struct Type *ret_t; 
      Param *params;
    } funct;
  };
} Type;

Type* get_type_int();
Type* get_type_flt();
void free_field(Field*);
void free_type(Type*);
bool type_eq(Type*, Type*);
bool get_field(Field*, const char*); 

#endif  // CMMC_TYPE_H

