#include <stdio.h>
#include "lexer.h"

int main() {
  Token t = get_token();
  while (t != kEOF) {
    printf("get %s at lineno %d.\n", token_name[t], lineno);
    if (t == kINT) printf("    : %d\n", token_int_val);
    if (t == kFLOAT) printf("    : %lf\n", token_float_val);
    if (t == kRELOP) printf("    : %s\n", token_str);
    if (t == kID) printf("    : %s\n", token_str);
    if (t == kTYPE) printf("    : %s\n", token_str);
    t = get_token();
  }
  printf("Here are %d error(s).\n", error_cnt);
  return 0;
}

