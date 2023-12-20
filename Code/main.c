#include <stdio.h>
#include "tester.h"

void gen_code(Irnode*, FILE*);

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
      if (error_cnt == 0) {
        firout = fopen("a.ir", "w");
        print_ir(irlist.head);
        fclose(firout);
        fill_off(irlist.head);
        if (argc > 2) {
          FILE *fs = fopen(argv[2], "w");
          gen_code(irlist.head, fs);
          fclose(fs);
        }
        free_ir(irlist.head);
      }
    }
    free_ast(q);
  }
  free_cst(p);
  fclose(fp);
  return 0;
}

