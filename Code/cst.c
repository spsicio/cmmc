#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cst.h"

Cstnode* alloc_lex_node(const char* name, Token type, int lineno) {
  Cstnode* p = (Cstnode*)malloc(sizeof(Cstnode));
  p->name = name;
  p->type = type;
  switch (type) {
    case kINT: p->int_val = token_int_val; break;
    case kFLOAT: p->float_val = token_float_val; break;
    case kID:
    case kRELOP:
    case kTYPE: strcpy(p->str_val, token_str); break;
    default: break;
  }
  p->lineno = lineno;
  p->chd_num = 0;
  return p;
}

Cstnode* alloc_syntax_node(const char* name, int lineno, int chd_num) {
  Cstnode* p = (Cstnode*)malloc(sizeof(Cstnode));
  p->name = name;
  p->type = kNAT;
  p->lineno = lineno;
  p->chd_num = chd_num;
  return p;
}

void print_cst(Cstnode *p, int indent) {
  if (p == NULL) return;
  for (int i = 0; i < indent; ++i) putchar(' ');
  if (p->chd_num == 0) {
    switch (p->type) {
      case kINT: printf("INT: %d\n", p->int_val); break;
      case kFLOAT: printf("FLOAT: %.6lf\n", p->float_val); break;
      case kID: printf("ID: %s\n", p->str_val); break;
      case kTYPE: printf("TYPE: %s\n", p->str_val); break; 
      default: printf("%s\n", p->name); break;
    }
  } else {
    printf("%s (%d)\n", p->name, p->lineno);
    for (int i = 0; i < p->chd_num; ++i) {
      print_cst(p->chd[i], indent+2);
    }
  }
}

void free_cst(Cstnode *p) {
  if (p == NULL) return;
  for (int i = 0; i < p->chd_num; ++i) {
    free_cst(p->chd[i]);
  }
  free(p);
}

