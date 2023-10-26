#include <stdio.h>
#include "tester.h"

void test_lexer() {
  Token t = get_token();
  while (t != kEOF) {
    if (t == kINT) printf("%2d: INT %d\n", lineno, token_int_val);
    else if (t == kFLOAT) printf("%2d: FLOAT %.6lf\n", lineno, token_float_val);
    //else if (t == kRELOP) printf("%2d: RELOP %s\n", lineno, token_str);
    else if (t == kEQ) printf("%2d: ==\n", lineno);
    else if (t == kNEQ) printf("%2d: !=\n", lineno);
    else if (t == kLT) printf("%2d: <\n", lineno);
    else if (t == kLE) printf("%2d: <=\n", lineno);
    else if (t == kGT) printf("%2d: >\n", lineno);
    else if (t == kGE) printf("%2d: >=\n", lineno);
    else if (t == kID) printf("%2d: ID %s\n", lineno, token_str);
    else if (t == kTYPE) printf("%2d: TYPE %s\n", lineno, token_str);
    else printf("%2d: %s\n", lineno, token_name[t]);
    t = get_token();
  }
  printf("Here are %d error(s).\n", error_cnt);
}

void test_parser_combinator() {
  Cstnode *p = NULL;
  read_token();
  p = parser_program();
  if (error_cnt == 0) print_cst(p, 0);
  free_cst(p);
}

void test_avl() {
  AVLNode *p = NULL;
  char c, s[32];
  while (~scanf(" %c %s", &c, s)) {
    if (c == 'i' || c == 'I') {
      p = avl_insert(p, s, NULL);
      print_avl(p, 0);
    } else {
      AVLNode *q = avl_find(p, s);
    }
  }
  free_avl(p);
}

void test_ast() {
  read_token();
  Cstnode *p = parser_program();
  if (error_cnt == 0) {
    Astnode *q = ast_prog(p);
    print_ast(q, 0);
    free_ast(q);
  }
  free_cst(p);
}

void test_semantic_checker() {
  read_token();
  Cstnode *p = parser_program();
  if (error_cnt == 0) {
    Astnode *q = ast_prog(p);
    sem_check(q);
    free_ast(q);
  }
  free_cst(p);
}

