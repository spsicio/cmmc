#ifndef CMMC_IR_H
#define CMMC_IR_H
#include <stdbool.h>
#include "lexer.h"
#include "util.h"
#include "ast.h"

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
  bool ref, deref;
} Opr;

#define IR(F) \
    /* BASIC */ \
    F(IR_ASN) \
    F(IR_ADD) \
    F(IR_SUB) \
    F(IR_MUL) \
    F(IR_DIV) \
    /* DECLARE */ \
    F(IR_DEC) \
    /* JUMP */ \
    F(IR_LBL) \
    F(IR_JMP) \
    F(IR_JCN) \
    /* FUNCTION */ \
    F(IR_FUN) \
    F(IR_PAR) \
    F(IR_RET) \
    F(IR_CAL) \
    F(IR_ARG) \
    /* EXTERN FUNCTION */ \
    F(IR_RED) \
    F(IR_RIT)

typedef enum {
  IR(F_LIST)
} IR_KIND;

typedef struct Irnode {
  struct Irnode *nxt;
  IR_KIND kind;
  Opr dst, src1, src2;
  Token op;
  char* fun_name;
} Irnode;

typedef void (*IrVisitT)(Irnode*, void*);
#define IR_VISIT_FUNPTR(KIND) IrVisitT VisitPtr_##KIND;
#define IR_MAKE_VISIT(KIND) static void Visit_##KIND(Irnode *p, void *arg)
#define IR_VISIT_FUNDEC(KIND) IR_MAKE_VISIT(KIND);

typedef struct IRVisitor {
  IR(IR_VISIT_FUNPTR)
} IRVisitor;

typedef struct Irlist {
  struct Irnode *head, *tail;
} Irlist;

typedef struct {
  Irlist list;
  Opr opr;
  bool cond;
} GenAttr;

extern int cnt_var;
extern int cnt_tmp;
extern int cnt_lbl;
Irlist link_ls(Irlist, Irlist);
Irlist gen_ir(Astnode*);
void print_ir(Irnode*);
void IrVisitorDispatch(IRVisitor*, Irnode*, void*);

#endif  // CMMC_IR_H

