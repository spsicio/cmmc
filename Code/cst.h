#ifndef CCMC_CST_H
#define CCMC_CST_H

#define MAX_CHD_NUM (7)

#include "lexer.h"

typedef struct Cstnode {
  const char* name;
  Token type;
  union {
    int int_val;
    float float_val;
    char str_val[MAX_TOKEN_LEN + 1];
  };
  int lineno;
  int chd_num; 
  struct Cstnode* chd[MAX_CHD_NUM];
} Cstnode;

Cstnode* alloc_lex_node(const char* name, Token type, int lineno);
Cstnode* alloc_syntax_node(const char* name, int lineno, int chd_num);
void print_cst(Cstnode*, int indent);
void free_cst(Cstnode*);

#endif  // CMMC_CST_H

