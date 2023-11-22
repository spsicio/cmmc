#include <stdio.h>
#include "ir.h"

FILE *firout;

static void print_opr(Opr p) {
  if (p.ref) fputc('&', firout);
  if (p.deref) fputc('*', firout);
  switch (p.kind) {
    case OPR_LIT: fprintf(firout, "#%d", p.litval); break;
    case OPR_VAR: fprintf(firout, "v%d", p.varno); break;
    case OPR_TMP: fprintf(firout, "t%d", p.tmpno); break;
    case OPR_LBL: fprintf(firout, "label%d", p.lblno); break;
    default: return;
  }
}

static void print_opt(Token op) {
  switch (op) {
    case kEQ: fprintf(firout, "=="); break;
    case kNEQ: fprintf(firout, "!="); break;
    case kLT: fprintf(firout, "<"); break;
    case kLE: fprintf(firout, "<="); break;
    case kGT: fprintf(firout, ">"); break;
    case kGE: fprintf(firout, ">="); break;
    case kPLUS: fprintf(firout, "+"); break;
    case kMINUS: fprintf(firout, "-"); break;
    case kSTAR: fprintf(firout, "*"); break;
    case kDIV: fprintf(firout, "/"); break;
    default: return;
  }
}

IR(IR_VISIT_FUNDEC)
VISITOR_DEF(IR, visitor)

void print_ir(Irnode *p) {
  while (p != NULL) {
    IrVisitorDispatch(&visitor, p, NULL);
    fputc('\n', firout);
    p = p->nxt;
  }
}

IR_MAKE_VISIT(IR_ASN) {
  print_opr(p->dst);
  fprintf(firout, " := ");
  print_opr(p->src1);
}

IR_MAKE_VISIT(IR_ARI) {
  print_opr(p->dst);
  fprintf(firout, " := ");
  print_opr(p->src1);
  fputc(' ', firout);
  print_opt(p->op);
  fputc(' ', firout);
  print_opr(p->src2);
}

IR_MAKE_VISIT(IR_DEC) {
  fprintf(firout, "DEC ");
  print_opr(p->dst);
  fprintf(firout, " %d", p->src1.litval);
}

IR_MAKE_VISIT(IR_LBL) {
  fprintf(firout, "LABEL ");
  print_opr(p->dst);
  fprintf(firout, " :");
}

IR_MAKE_VISIT(IR_JMP) {
  fprintf(firout, "GOTO ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_JCN) {
  fprintf(firout, "IF ");
  print_opr(p->src1);
  fputc(' ', firout);
  print_opt(p->op);
  fputc(' ', firout);
  print_opr(p->src2);
  fprintf(firout, " GOTO ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_FUN) {
  fprintf(firout, "FUNCTION %s :", p->fun_name);
}

IR_MAKE_VISIT(IR_PAR) {
  fprintf(firout, "PARAM ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_RET) {
  fprintf(firout, "RETURN ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_CAL) {
  print_opr(p->dst);
  fprintf(firout, " := CALL %s", p->fun_name);
}

IR_MAKE_VISIT(IR_ARG) {
  fprintf(firout, "ARG ");
  print_opr(p->dst);
} 

IR_MAKE_VISIT(IR_RED) {
  fprintf(firout, "READ ");
  print_opr(p->dst);
}

IR_MAKE_VISIT(IR_RIT) {
  fprintf(firout, "WRITE ");
  print_opr(p->dst);
}

