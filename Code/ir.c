#include <stdlib.h>
#include "ir.h"

Irnode* alloc_irnode(IR_KIND kind) {
  Irnode *p = (Irnode*) malloc(sizeof(Irnode));
  p->kind = kind;
  p->nxt = NULL;
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

