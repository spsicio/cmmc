#include <stdio.h>
#include "test.h"

int main(int argc, char **argv) {
  if (argc <= 1) return 1;
  fp = fopen(argv[1], "r");
  // test_lexer();
  test_parser_combinator();
  return 0;
}

