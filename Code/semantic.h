#ifndef CMMC_SEMANTIC_H
#define CMMC_SEMANTIC_H

#include "ast.h"
#include "symtab.h"

#define SEM_ERR(F) \
    F(UNDEFINED_ERR) \
    F(VAR_UNDEF) \
    F(FUN_UNDEF) \
    F(VAR_REDEF) \
    F(FUN_REDEF) \
    F(ASGN_MISMATCH) \
    F(ASGN_2RVALUE) \
    F(OPR_MISMATCH) \
    F(RET_MISMATCH) \
    F(ARG_MISMATCH) \
    F(ACCESS_NOT_ARRAY) \
    F(CALL_NOT_FUNCT) \
    F(ACCESS_ARRAY_NINT) \
    F(ACCESS_NOT_STRCT) \
    F(ACCESS_FIELD_UNDEF) \
    F(FIELD_REDEF) \
    F(STRCT_REDEF) \
    F(STRCT_UNDEF)

typedef enum {
  SEM_ERR(F_LIST) 
} SEM_ERRNO;

Type* sem_check(Astnode*);

#endif  // CMMC_SEMANTIC_H

