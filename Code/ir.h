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
    F(IR_ARI) \
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
  const char* fun_name;
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

extern FILE *firout;
Irnode* alloc_ir_f(IR_KIND kind, const char*);
Irnode* alloc_ir_d(IR_KIND kind, Opr);
Irnode* alloc_ir_df(IR_KIND kind, Opr, const char*);
Irnode* alloc_ir_ds(IR_KIND kind, Opr, Opr);
Irnode* alloc_ir_dsso(IR_KIND kind, Opr, Opr, Opr, Token);
void free_ir(Irnode*);
Irlist link_ls(Irlist, Irlist);
void ir_push_f(Irlist*, Irnode*);
void ir_push_b(Irlist*, Irnode*);
Irlist gen_ir(Astnode*, void*);
void print_ir(Irnode*);
void IrVisitorDispatch(IRVisitor*, Irnode*, void*);

#endif  // CMMC_IR_H

