#include <stdio.h>
#include "ir.h"

static void print_opr(Opr p) {
  if (p.ref) putchar('&');
  if (p.deref) putchar('*');
  switch (p.kind) {
    case OPR_LIT: printf("#%d", p.litval); break;
    case OPR_VAR: printf("v%d", p.varno); break;
    case OPR_TMP: printf("t%d", p.tmpno); break;
    case OPR_LBL: printf("label%d", p.lblno); break;
    default: return;
  }
}

IR(IR_VISIT_FUNDEC)
VISITOR_DEF(IR, visitor)

void print_ir(Irnode *p) {
  while (p != NULL) {
    IrVisitorDispatch(&visitor, p, NULL);
    putchar('\n');
    p = p->nxt;
  }
}

IR_MAKE_VISIT(IR_ASN) {
  print_opr(p->dst);
  printf(" := ");
  print_opr(p->src1);
}

IR_MAKE_VISIT(IR_ADD) {
  print_opr(p->dst);
  printf(" := ");
  print_opr(p->src1);
  printf(" + ");
  print_opr(p->src2);
}

IR_MAKE_VISIT(IR_SUB) {
  print_opr(p->dst);
  printf(" := ");
  print_opr(p->src1);
  printf(" - ");
  print_opr(p->src2);
}

IR_MAKE_VISIT(IR_MUL) {
  print_opr(p->dst);
  printf(" := ");
  print_opr(p->src1);
  printf(" * ");
  print_opr(p->src2);
}

IR_MAKE_VISIT(IR_DIV) {
  print_opr(p->dst);
  printf(" := ");
  print_opr(p->src1);
  printf(" / ");
  print_opr(p->src2);
}

IR_MAKE_VISIT(IR_DEC) {
  printf("DEC ");
  print_opr(p->dst);
  printf(" %d", p->src1.litval);
}

IR_MAKE_VISIT(IR_LBL) {
  printf("LABEL ");
  print_opr(p->dst);
  printf(" :");
}

IR_MAKE_VISIT(IR_JMP) {
  printf("GOTO ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_JCN) {
  printf("IF ");
  print_opr(p->src1);
  printf(" %s ", token_name[p->op]); 
  print_opr(p->src2);
  printf(" GOTO ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_FUN) {
  printf("FUNCTION %s :", p->fun_name);
}

IR_MAKE_VISIT(IR_PAR) {
  printf("PARAM ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_RET) {
  printf("RETURN ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_CAL) {
  print_opr(p->dst);
  printf(" := CALL %s", p->fun_name);
}

IR_MAKE_VISIT(IR_ARG) {
  printf("ARG ");
  print_opr(p->dst);
} 

IR_MAKE_VISIT(IR_RED) {
  printf("READ ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_RIT) {
  printf("WRITE ");
  print_opr(p->dst);
}

