#include <stdio.h>
#include "tester.h"

int main(int argc, char **argv) {
  if (argc <= 1) return 1;
  fp = fopen(argv[1], "r");
  //test_lexer();
  //test_parser_combinator();
  //test_ast();
  test_semantic_checker();
  return 0;
}

