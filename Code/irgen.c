#include <stdio.h>
#include "symtab.h"
#include "ir.h"

int cnt_var = 0;
int cnt_tmp = 0;
int cnt_lbl = 0;

Opr make_litval(int litval) {
  return (Opr) {
      .kind = OPR_LIT,
      .litval = litval,
      .ref = false,
      .deref = false, };
}

Opr lookup_var(const char* name) {
  Syment *ent = symtab_lookup(name);
  if (ent->varno == 0) {
    ++cnt_var;
    ent->varno = cnt_var;
  }
  return (Opr) {
      .kind = OPR_VAR,
      .tmpno = ent->varno,
      .ref = false,
      .deref = false, };
}

Opr new_tmpvar() {
  ++cnt_tmp;
  return (Opr) {
      .kind = OPR_TMP,
      .tmpno = cnt_tmp,
      .ref = false,
      .deref = false, };
}

Opr new_label() {
  ++cnt_lbl;
  return (Opr) {
      .kind = OPR_LBL,
      .lblno = cnt_lbl,
      .ref = false,
      .deref = false, };
}

typedef struct {
  Irlist list;
  Opr opr;
  bool cond;
} GenAttr;

AST(AST_VISIT_FUNDEC)
VISITOR_DEF(AST, visitor)

Irlist gen_ir(Astnode *p, void *arg) {
  if (p == NULL) return (Irlist) {NULL, NULL};
  if (arg == NULL) {
    GenAttr attr = (GenAttr) {
        .list = {NULL, NULL},
        .opr = {OPR_TMP, 0, false, false},
        .cond = false, };
    arg = &attr;
  }
  AstVisitorDispatch(&visitor, p, arg);
  return ((GenAttr*) arg)->list;
}

AST_MAKE_VISIT(EXP_ID) {
  GenAttr *attr = arg;
  Irnode *node_asn = alloc_irnode(IR_ASN);
  node_asn->dst = attr->opr;
  node_asn->src1 = lookup_var(p->exp_id.name);
  ir_push_b(&(attr->list), node_asn);
}

AST_MAKE_VISIT(EXP_INT) {
  GenAttr *attr = arg;
  Irnode *node_asn = alloc_irnode(IR_ASN);
  node_asn->dst = attr->opr;
  node_asn->src1 = make_litval(p->exp_int.val);
  ir_push_b(&(attr->list), node_asn);
}

AST_MAKE_VISIT(EXP_FLT) {  // NOT SUPPORTED
  ++error_cnt;
  printf("Cannot translate: float.\n");
}

AST_MAKE_VISIT(EXP_UOP) {
  GenAttr *attr = arg;
  if (p->exp_uop.uop == kMINUS) {
    Opr opr_sub = new_tmpvar();
    GenAttr subattr = (GenAttr) {
        .list = {NULL, NULL},
        .opr = opr_sub,
        .cond = false, };
    attr->list = gen_ir(p->exp_uop.rhs, &subattr);
    Irnode *node_neg = alloc_irnode(IR_ARI);
    node_neg->dst = attr->opr;
    node_neg->src1 = make_litval(0);
    node_neg->src2 = opr_sub;
    node_neg->op = kMINUS;
    ir_push_b(&(attr->list), node_neg);
  } else {
    // TODO
  }
}

AST_MAKE_VISIT(EXP_BOP) {
  GenAttr *attr = arg;
  if (p->exp_bop.bop == kASSIGNOP) {
  } else if (p->exp_bop.bop >= kPLUS && p->exp_bop.bop <= kDIV) {
    Opr opr_l = new_tmpvar();
    Opr opr_r = new_tmpvar();
    GenAttr subattr_l = (GenAttr) {
        .list = {NULL, NULL},
        .opr = opr_l,
        .cond = false, };
    GenAttr subattr_r = (GenAttr) {
        .list = {NULL, NULL},
        .opr = opr_r,
        .cond = false, };
    Irlist ls_l = gen_ir(p->exp_bop.lhs, &subattr_l);
    Irlist ls_r = gen_ir(p->exp_bop.rhs, &subattr_r);
    Irnode *node_ari = alloc_irnode(IR_ARI);
    node_ari->dst = attr->opr;
    node_ari->src1 = opr_l;
    node_ari->src2 = opr_r;
    node_ari->op = p->exp_bop.bop;
    attr->list = link_ls(ls_l, ls_r);
    ir_push_b(&(attr->list), node_ari);
  } else if (p->exp_bop.bop >= kAND && p->exp_bop.bop <= kOR) {
    // TODO
  } else if (p->exp_bop.bop >= kEQ && p->exp_bop.bop <= kGE) {
    // TODO
  }
}

AST_MAKE_VISIT(EXP_ARRAY) {
  GenAttr *attr = arg;
  // TODO
}

AST_MAKE_VISIT(EXP_FIELD) {  // NOT SUPPORTED
  ++error_cnt;
  printf("Cannot translate: float.\n");
}

AST_MAKE_VISIT(EXP_CALL) {
  GenAttr *attr = arg;
  // TODO
}

AST_MAKE_VISIT(STMT_EXP) {
  GenAttr *attr = arg;
  attr->list = gen_ir(p->stmt_exp.exp, NULL);
}

AST_MAKE_VISIT(STMT_IF) {
  // TODO
}

AST_MAKE_VISIT(STMT_WHILE) {
  // TODO
}

AST_MAKE_VISIT(STMT_RET) {
  GenAttr *attr = arg;
  Opr opr_ret = new_tmpvar();
  GenAttr subattr = (GenAttr) {
      .list = {NULL, NULL},
      .opr = opr_ret,
      .cond = false, };
  attr->list = gen_ir(p->stmt_ret.exp, &subattr);
  Irnode *node_ret = alloc_irnode(IR_RET);
  node_ret->dst = opr_ret;
  ir_push_b(&(attr->list), node_ret);
}

AST_MAKE_VISIT(STMT_COMP) {
  GenAttr *attr = arg;
  for (Astnode *q = p->stmt_comp.defs; q != NULL; q = q->nxt) gen_ir(q, NULL);
  for (Astnode *q = p->stmt_comp.stmts; q != NULL; q = q->nxt) {
    Irlist ls = gen_ir(q, NULL);
    attr->list = link_ls(attr->list, ls);
  }
}

AST_MAKE_VISIT(CONSTR_SPEC) {}

AST_MAKE_VISIT(DEF_TYP) {}

AST_MAKE_VISIT(DEF_VAR) {
  GenAttr *attr = arg;
  Syment *ent = symtab_insert(p->def_var.name);
  ent->kind = kVAR;
  ent->type = &(p->type);
  if (attr->cond) {  // PARAMETER
    ++cnt_var;
    ent->varno = cnt_var;
  } else {
    if (ent->type->kind == kARRAY) {  // DEC
      ++cnt_var;
      ent->varno = cnt_var;
      Irnode *node_dec = alloc_irnode(IR_DEC);
      node_dec->dst = lookup_var(p->def_var.name);
      node_dec->src1 = make_litval(p->type.array.siz);
      ir_push_b(&(attr->list), node_dec);
    } else {  // LAZY_LABELLING
      ent->varno = 0;
    }
  }
}

AST_MAKE_VISIT(DEF_FUN) {
  GenAttr *attr = arg;
  Irnode *node_fun = alloc_irnode(IR_FUN);
  node_fun->fun_name = p->def_fun.name;
  ir_push_b(&(attr->list), node_fun);
  for (Astnode *q = p->def_fun.params; q != NULL; q = q->nxt) {
    GenAttr subattr = (GenAttr) {
        .list = {NULL, NULL},
        .opr = {OPR_TMP, 0, false, false},
        .cond = true, };
    gen_ir(q, &subattr);
    Irnode *node_par = alloc_irnode(IR_PAR);
    node_par->dst = lookup_var(q->def_var.name);
    ir_push_b(&(attr->list), node_par);
  }
  Irlist ls = gen_ir(p->def_fun.body, NULL);
  attr->list = link_ls(attr->list, ls);
}

AST_MAKE_VISIT(LS_PROG) {
  GenAttr *attr = arg;
  symtab_push_scope();
  for (Astnode *q = p->ls_prog.defs; q != NULL; q = q->nxt) {
    Irlist ls = gen_ir(q, NULL);
    attr->list = link_ls(attr->list, ls);
  }
  symtab_pop_scope();
}

