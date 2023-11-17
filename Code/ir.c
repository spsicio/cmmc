#include "ir.h"

int cnt_var = 0;
int cnt_tmp = 0;
int cnt_lbl = 0;

Irlist link_ls(Irlist a, Irlist b) {
  if (a.head == NULL) return b;
  if (b.head == NULL) return a;
  Irlist c = (Irlist) {a.head, b.tail};
  a.tail->nxt = b.head;
  return c;
}

void IrVisitorDispatch(IRVisitor *visitor, Irnode *p, void *arg) {
  if (p == NULL) return;
  switch (p->kind) {
    IR(F_DISPATCH)
  }
}

