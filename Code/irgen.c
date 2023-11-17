#include <stdio.h>
#include "symtab.h"
#include "ir.h"

AST(AST_VISIT_FUNDEC)
VISITOR_DEF(AST, visitor)

Irlist gen_ir(Astnode *p) {
  if (p == NULL) return (Irlist) {NULL, NULL};
  GenAttr attr = (GenAttr) {
      .list.head = NULL,
      .list.tail = NULL,
      .opr.kind = OPR_TMP,
      .opr.tmpno = 0,
      .cond = false, };
  AstVisitorDispatch(&visitor, p, &attr);
  return attr.list;
}

AST_MAKE_VISIT(EXP_ID) {
}

AST_MAKE_VISIT(EXP_INT) {
}

AST_MAKE_VISIT(EXP_FLT) {  // NOT SUPPORTED
  ++error_cnt;
  printf("Cannot translate: float.\n");
}

AST_MAKE_VISIT(EXP_UOP) {
  GenAttr *attr = arg;
  if (p->exp_uop.uop == kMINUS) {
  } else {
  }
}

AST_MAKE_VISIT(EXP_BOP) {
  GenAttr *attr = arg;
  if (p->exp_bop.bop == kASSIGNOP) {
  } else if (p->exp_bop.bop >= kPLUS && p->exp_bop.bop <= kDIV) {
  } else if (p->exp_bop.bop >= kAND && p->exp_bop.bop <= kOR) {
  } else if (p->exp_bop.bop >= kEQ && p->exp_bop.bop <= kGE) {
  }
}

AST_MAKE_VISIT(EXP_ARRAY) {
  GenAttr *attr = arg;
}

AST_MAKE_VISIT(EXP_FIELD) {  // NOT SUPPORTED
  ++error_cnt;
  printf("Cannot translate: float.\n");
}

AST_MAKE_VISIT(EXP_CALL) {
  GenAttr *attr = arg;
}

AST_MAKE_VISIT(STMT_EXP) {
  gen_ir(p->stmt_exp.exp);
}

AST_MAKE_VISIT(STMT_IF) {
}

AST_MAKE_VISIT(STMT_WHILE) {
}

AST_MAKE_VISIT(STMT_RET) {
}

AST_MAKE_VISIT(STMT_COMP) {
  GenAttr *attr = arg;
  for (Astnode *q = p->stmt_comp.defs; q != NULL; q = q->nxt) gen_ir(q);
  for (Astnode *q = p->stmt_comp.stmts; q != NULL; q = q->nxt) {
    Irlist ls = gen_ir(q);
    attr->list = link_ls(attr->list, ls);
  }
}

AST_MAKE_VISIT(CONSTR_SPEC) {}

AST_MAKE_VISIT(DEF_TYP) {}

AST_MAKE_VISIT(DEF_VAR) {
  Syment *ent = symtab_insert(p->def_var.name);
  ent->kind = kVAR;
  ent->type = &(p->type);
  ent->varno = 0;
}

AST_MAKE_VISIT(DEF_FUN) {
}

AST_MAKE_VISIT(LS_PROG) {
  GenAttr *attr = arg;
  symtab_push_scope();
  for (Astnode *q = p->ls_prog.defs; q != NULL; q = q->nxt) {
    Irlist ls = gen_ir(q);
    attr->list = link_ls(attr->list, ls);
  }
  symtab_pop_scope();
}

