#include <stdlib.h>
#include "ir.h"

Irnode* alloc_ir_f(IR_KIND kind, const char *fun_name) {
  Irnode *p = (Irnode*) malloc(sizeof(Irnode));
  p->kind = kind;
  p->nxt = NULL;
  p->fun_name = fun_name;
  return p;
}

Irnode* alloc_ir_d(IR_KIND kind, Opr dst) {
  Irnode *p = (Irnode*) malloc(sizeof(Irnode));
  p->kind = kind;
  p->nxt = NULL;
  p->dst = dst;
  return p;
}

Irnode* alloc_ir_df(IR_KIND kind, Opr dst, const char *fun_name) {
  Irnode *p = (Irnode*) malloc(sizeof(Irnode));
  p->kind = kind;
  p->nxt = NULL;
  p->dst = dst;
  p->fun_name = fun_name;
  return p;
}

Irnode* alloc_ir_ds(IR_KIND kind, Opr dst, Opr src1) {
  Irnode *p = (Irnode*) malloc(sizeof(Irnode));
  p->kind = kind;
  p->nxt = NULL;
  p->dst = dst;
  p->src1 = src1;
  return p;
}

Irnode* alloc_ir_dsso(IR_KIND kind, Opr dst, Opr src1, Opr src2, Token op) {
  Irnode *p = (Irnode*) malloc(sizeof(Irnode));
  p->kind = kind;
  p->nxt = NULL;
  p->dst = dst;
  p->src1 = src1;
  p->src2 = src2;
  p->op = op;
  return p;
}

Irlist link_ls(Irlist a, Irlist b) {
  if (a.head == NULL) return b;
  if (b.head == NULL) return a;
  Irlist c = (Irlist) {a.head, b.tail};
  a.tail->nxt = b.head;
  return c;
}

void ir_push_b(Irlist *ls, Irnode *p) {
  if (ls->tail == NULL) ls->head = p;
  else ls->tail->nxt = p;
  ls->tail = p;
}

void ir_push_f(Irlist *ls, Irnode *p) {
  if (ls->head == NULL) ls->tail = p;
  p->nxt = ls->head; 
  ls->head = p;
}

void IrVisitorDispatch(IRVisitor *visitor, Irnode *p, void *arg) {
  if (p == NULL) return;
  switch (p->kind) {
    IR(F_DISPATCH)
  }
}

