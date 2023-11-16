#ifndef CMMC_IR_H
#define CMMC_IR_H
#include <stdbool.h>

typedef enum {
  OPR_LIT,  // #1
  OPR_VAR,  // v1
  OPR_TMP,  // t1
  OPR_LBL,  // label1
} OPR_KIND;

typedef struct {
  OPR_KIND kind;
  union {
    int litval;
    int varno;
    int tmpno;
    int lblno;
  };
} Opr;

typedef struct Irnode {
  struct Irnode *nxt;
} Irnode;

typedef struct Irlist {
  struct Irnode *head, *tail;
} Irlist;

typedef struct {
  Irlist list;
  Opr opr;
  bool cond;
} GenAttr;

Irlist gen_ir(Astnode*);

#endif  // CMMC_IR_H

