#ifndef CCMC_AST_H
#define CCMC_AST_H

#define MAX_CHD_NUM (7)

#include "lexer.h"

typedef struct Astnode {
  const char* name;
  Token type;
  union {
    int int_val;
    float float_val;
    char str_val[MAX_TOKEN_LEN + 1];
  };
  int lineno;
  int chd_num; 
  struct Astnode* chd[MAX_CHD_NUM];
} Astnode;

Astnode* alloc_lex_node(const char* name, Token type, int lineno);
Astnode* alloc_syntax_node(const char* name, int lineno, int chd_num);
void print_ast(Astnode*, int indent);
void free_ast(Astnode*);

#endif  // CMMC_AST_H

