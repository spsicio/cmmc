#include <stdlib.h>
#include "avl.h"
#include "ir.h"
#include "oprtab.h"

AVLNode *root = NULL;

Oprent* oprtab_access(const char *name) {
  AVLNode *res = avl_find(root, name);
  if (res != NULL) return res->ent;
  Oprent *ent = (Oprent*) malloc(sizeof(Oprent));
  ent->siz = 4;
  ent->off = 0;
  root = avl_insert(root, name, ent); 
  return ent;
}

IR(IR_VISIT_FUNDEC)
VISITOR_DEF(IR, visitor)

#define local_off (fun_ent->off)
#define param_off (fun_ent->siz)
static Oprent *fun_ent = NULL;

void fill_off(Irnode *p) {
  while (p != NULL) {
    IrVisitorDispatch(&visitor, p, NULL);
    p = p->nxt;
  }
}

static void set_off(Opr opr, int *off, int dir) {
  symstr name;
  if (opr.kind == OPR_VAR) sprintf(name, "v%d", opr.varno);
  else if (opr.kind == OPR_TMP) sprintf(name, "t%d", opr.tmpno);
  else return;
  Oprent *ent = oprtab_access(name);
  if (ent->off == 0) {
    *off += ent->siz * dir;
    ent->off = *off - dir * 4;
    //printf("%s %d\n", name, ent->off);
  }
}

IR_MAKE_VISIT(IR_ASN) {
  set_off(p->dst, &local_off, -1);
  set_off(p->src1, &local_off, -1);
}

IR_MAKE_VISIT(IR_ARI) {
  set_off(p->dst, &local_off, -1);
  set_off(p->src1, &local_off, -1);
  set_off(p->src2, &local_off, -1);
}

IR_MAKE_VISIT(IR_DEC) {
  set_off(p->dst, &local_off, -1);
}

IR_MAKE_VISIT(IR_LBL) {}

IR_MAKE_VISIT(IR_JMP) {}

IR_MAKE_VISIT(IR_JCN) {
  set_off(p->src1, &local_off, -1);
  set_off(p->src2, &local_off, -1);
}

IR_MAKE_VISIT(IR_FUN) {
  fun_ent = oprtab_access(p->fun_name);
  param_off = +8;
  local_off = -4;
}

IR_MAKE_VISIT(IR_PAR) {
  set_off(p->dst, &param_off, +1);
}

IR_MAKE_VISIT(IR_RET) {
  set_off(p->dst, &local_off, -1);
}

IR_MAKE_VISIT(IR_CAL) {}

IR_MAKE_VISIT(IR_ARG) {
  set_off(p->dst, &local_off, -1);
}

IR_MAKE_VISIT(IR_RED) {
  set_off(p->dst, &local_off, -1);
}

IR_MAKE_VISIT(IR_RIT) {
  set_off(p->dst, &local_off, -1);
}

