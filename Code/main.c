#include <stdio.h>
#include "tester.h"

int main(int argc, char **argv) {
  if (argc <= 1) return 1;
  fp = fopen(argv[1], "r");
  read_token();
  Cstnode *p = parser_program();
  if (error_cnt == 0) {
    Astnode *q = ast_prog(p);
    sem_check(q);
    if (error_cnt == 0) {
      Irlist irlist = gen_ir(q, NULL);
      print_ir(irlist.head);
    }
    free_ast(q);
  }
  free_cst(p);
  return 0;
}

