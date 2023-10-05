#include <stdio.h>
#include "parser.h"

void test_lexer() {
  Token t = get_token();
  while (t != kEOF) {
    if (t == kINT) printf("%2d: INT %d\n", lineno, token_int_val);
    else if (t == kFLOAT) printf("%2d: FLOAT %.6lf\n", lineno, token_float_val);
    else if (t == kRELOP) printf("%2d: RELOP %s\n", lineno, token_str);
    else if (t == kID) printf("%2d: ID %s\n", lineno, token_str);
    else if (t == kTYPE) printf("%2d: TYPE %s\n", lineno, token_str);
    else printf("%2d: %s\n", lineno, token_name[t]);
    t = get_token();
  }
  printf("Here are %d error(s).\n", error_cnt);
}

void test_parser_combinator() {
  Astnode *p = NULL;
  read_token();
  // p = parser_exp();
  p = parser_program();
  print_ast(p, 0);
  free_ast(p);
}

