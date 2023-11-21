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
  Opr lbl;
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

static void gen_cond_ari(Irlist *ls, Opr val, Opr opr, Opr lbl) {
  Irnode *node_jcn = alloc_irnode(IR_JCN);
  Irnode *node_jmp = alloc_irnode(IR_JMP);
  node_jcn -> dst = opr;
  node_jcn -> src1 = val;
  node_jcn -> src2 = make_litval(0);
  node_jcn -> op = kNEQ;
  node_jmp -> dst = lbl;
  ir_push_b(ls, node_jcn);
  ir_push_b(ls, node_jmp);
}

static void gen_exp_bool_pre(Irlist *ls, Opr opr) {
  Irnode *node_asn_f = alloc_irnode(IR_ASN);
  node_asn_f->dst = opr;
  node_asn_f->src1 = make_litval(0);
  ir_push_b(ls, node_asn_f);
}

static void gen_exp_bool_post(Irlist *ls, Opr opr, Opr lbl_t, Opr lbl_f) {
  Irnode *node_lbl_t = alloc_irnode(IR_LBL);
  Irnode *node_lbl_f = alloc_irnode(IR_LBL);
  Irnode *node_asn_t = alloc_irnode(IR_ASN);
  node_lbl_t->dst = lbl_t;
  node_lbl_f->dst = lbl_f;
  node_asn_t->dst = opr;
  node_asn_t->src1 = make_litval(1);
  ir_push_b(ls, node_lbl_t);
  ir_push_b(ls, node_asn_t);
  ir_push_b(ls, node_lbl_f);
}

AST_MAKE_VISIT(EXP_ID) {
  GenAttr *attr = arg;
  Opr opr_id = lookup_var(p->exp_id.name);
  if (attr->cond) {
    gen_cond_ari(&(attr->list), opr_id, attr->opr, attr->lbl);
  } else {
    if (attr->opr.litval != 0) --cnt_tmp;
    attr->opr = opr_id;
  }
}

AST_MAKE_VISIT(EXP_INT) {
  GenAttr *attr = arg;
  Opr opr_int = make_litval(p->exp_int.val);
  if (attr->cond) {
    gen_cond_ari(&(attr->list), opr_int, attr->opr, attr->lbl);
  } else {
    if (attr->opr.litval != 0) --cnt_tmp;
    attr->opr = opr_int;
  }
}

AST_MAKE_VISIT(EXP_FLT) {  // NOT SUPPORTED
  ++error_cnt;
  printf("Cannot translate: float.\n");
}

AST_MAKE_VISIT(EXP_UOP) {
  GenAttr *attr = arg;
  if (p->exp_uop.uop == kMINUS) {
    GenAttr subattr = (GenAttr) {
        .list = {NULL, NULL},
        .opr = new_tmpvar(),
        .cond = false, };
    attr->list = gen_ir(p->exp_uop.rhs, &subattr);
    Irnode *node_neg = alloc_irnode(IR_ARI);
    node_neg->dst = attr->cond ? new_tmpvar() : attr->opr;
    node_neg->src1 = make_litval(0);
    node_neg->src2 = subattr.opr;
    node_neg->op = kMINUS;
    ir_push_b(&(attr->list), node_neg);
    if (attr->cond)
      gen_cond_ari(&(attr->list), node_neg->dst, attr->opr, attr->lbl);
  } else {
    Opr save = attr->opr;
    if (!(attr->cond)) {
      gen_exp_bool_pre(&(attr->list), save);
      attr->opr = new_label();
      attr->lbl = new_label();
    }
    GenAttr subattr = (GenAttr) {
          .list = {NULL, NULL},
          .opr = attr->lbl,
          .cond = true,
          .lbl = attr->opr, };
    attr->list = gen_ir(p->exp_bop.rhs, &subattr);
    if (!(attr->cond)) {
      gen_exp_bool_post(&(attr->list), save, attr->opr, attr->lbl);
      attr->opr = save;
    }
  }
}

AST_MAKE_VISIT(EXP_BOP) {
  GenAttr *attr = arg;
  if (p->exp_bop.bop == kASSIGNOP) {
    if (p->type.kind == kINT_t) {
      GenAttr subattr = (GenAttr) {
          .list = {NULL, NULL},
          .opr = new_tmpvar(),
          .cond = false, };
      attr->list = gen_ir(p->exp_bop.rhs, &subattr);
      Irnode *node_asn = alloc_irnode(IR_ASN);
      node_asn->dst = lookup_var(p->exp_bop.lhs->exp_id.name);
      node_asn->src1 = subattr.opr;
      ir_push_b(&(attr->list), node_asn);
    } else {
      // TODO
    }
  } else if (p->exp_bop.bop >= kPLUS && p->exp_bop.bop <= kDIV) {
    GenAttr subattr_l = (GenAttr) {
        .list = {NULL, NULL},
        .opr = new_tmpvar(),
        .cond = false, };
    Irlist ls_l = gen_ir(p->exp_bop.lhs, &subattr_l);
    GenAttr subattr_r = (GenAttr) {
        .list = {NULL, NULL},
        .opr = new_tmpvar(),
        .cond = false, };
    Irlist ls_r = gen_ir(p->exp_bop.rhs, &subattr_r);
    Irnode *node_ari = alloc_irnode(IR_ARI);
    node_ari->dst = attr->cond ? new_tmpvar() : attr->opr;
    node_ari->src1 = subattr_l.opr;
    node_ari->src2 = subattr_r.opr;
    node_ari->op = p->exp_bop.bop;
    attr->list = link_ls(ls_l, ls_r);
    ir_push_b(&(attr->list), node_ari);
    if (attr->cond)
      gen_cond_ari(&(attr->list), node_ari->dst, attr->opr, attr->lbl);
  } else if (p->exp_bop.bop >= kAND && p->exp_bop.bop <= kOR) {
    Opr save = attr->opr;
    if (!(attr->cond)) {
      gen_exp_bool_pre(&(attr->list), save);
      attr->opr = new_label();
      attr->lbl = new_label();
    }
    Opr lbl_m = new_label();
    GenAttr subattr_l = (GenAttr) {
        .list = {NULL, NULL},
        .opr = p->exp_bop.bop == kAND ? lbl_m : attr->opr,
        .cond = true,
        .lbl = p->exp_bop.bop == kAND ? attr->lbl : lbl_m, };
    GenAttr subattr_r = (GenAttr) {
        .list = {NULL, NULL},
        .opr = attr->opr,
        .cond = true,
        .lbl = attr->lbl, };
    Irlist ls_l = gen_ir(p->exp_bop.lhs, &subattr_l);
    Irlist ls_r = gen_ir(p->exp_bop.rhs, &subattr_r);
    Irnode *node_lbl_m = alloc_irnode(IR_LBL);
    node_lbl_m->dst = lbl_m;
    attr->list = link_ls(attr->list, ls_l);
    ir_push_b(&(attr->list), node_lbl_m);
    attr->list = link_ls(attr->list, ls_r);
    if (!(attr->cond)) {
      gen_exp_bool_post(&(attr->list), save, attr->opr, attr->lbl);
      attr->opr = save;
    }
  } else if (p->exp_bop.bop >= kEQ && p->exp_bop.bop <= kGE) {
    Opr save = attr->opr;
    if (!(attr->cond)) {
      gen_exp_bool_pre(&(attr->list), save);
      attr->opr = new_label();
      attr->lbl = new_label();
    }
    GenAttr subattr_l = (GenAttr) {
        .list = {NULL, NULL},
        .opr = new_tmpvar(),
        .cond = false, };
    Irlist ls_l = gen_ir(p->exp_bop.lhs, &subattr_l);
    GenAttr subattr_r = (GenAttr) {
        .list = {NULL, NULL},
        .opr = new_tmpvar(),
        .cond = false, };
    Irlist ls_r = gen_ir(p->exp_bop.rhs, &subattr_r);
    Irnode *node_jcn = alloc_irnode(IR_JCN);
    Irnode *node_jmp = alloc_irnode(IR_JMP);
    node_jcn -> dst = attr->opr;
    node_jcn -> src1 = subattr_l.opr;
    node_jcn -> src2 = subattr_r.opr;
    node_jcn -> op = p->exp_bop.bop;
    node_jmp -> dst = attr->lbl;
    attr->list = link_ls(attr->list, ls_l);
    attr->list = link_ls(attr->list, ls_r);
    ir_push_b(&(attr->list), node_jcn);
    ir_push_b(&(attr->list), node_jmp);
    if (!(attr->cond)) {
      gen_exp_bool_post(&(attr->list), save, attr->opr, attr->lbl);
      attr->opr = save;
    }
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
  Irlist ls_init = (Irlist) {NULL, NULL};
  Irnode *node_cal = alloc_irnode(IR_CAL);
  node_cal->dst = attr->opr;
  node_cal->fun_name = p->exp_call.funct;
  ir_push_f(&(attr->list), node_cal);
  for (Astnode *q = p->exp_call.args; q != NULL; q = q->nxt) {
    GenAttr subattr = (GenAttr) {
        .list = {NULL, NULL},
        .opr = new_tmpvar(),
        .cond = false, };
    Irlist ls = gen_ir(q, &subattr);
    ls_init = link_ls(ls_init, ls);
    Irnode *node_arg = alloc_irnode(IR_ARG);
    node_arg -> dst = subattr.opr;
    if (q->type.kind == kARRAY) node_arg->dst.ref = true;
    ir_push_f(&(attr->list), node_arg);
  }
  attr->list = link_ls(ls_init, attr->list);
}

AST_MAKE_VISIT(STMT_EXP) {
  GenAttr *attr = arg;
  attr->list = gen_ir(p->stmt_exp.exp, NULL);
}

AST_MAKE_VISIT(STMT_IF) {
  GenAttr *attr = arg;
  if (p->stmt_if.brnch0 != NULL) {
    Opr lbl_t = new_label();
    Opr lbl_f = new_label();
    Opr lbl_e = new_label();
    GenAttr subattr = (GenAttr) {
        .list = {NULL, NULL},
        .opr = lbl_t,
        .cond = true,
        .lbl = lbl_f, };
    Irlist ls_c = gen_ir(p->stmt_if.cond, &subattr);
    Irlist ls_t = gen_ir(p->stmt_if.brnch1, NULL);
    Irlist ls_f = gen_ir(p->stmt_if.brnch0, NULL);
    Irnode *node_lbl_t = alloc_irnode(IR_LBL);
    Irnode *node_lbl_f = alloc_irnode(IR_LBL);
    Irnode *node_lbl_e = alloc_irnode(IR_LBL);
    Irnode *node_jmp = alloc_irnode(IR_JMP);
    node_lbl_t -> dst = lbl_t;
    node_lbl_f -> dst = lbl_f;
    node_lbl_e -> dst = lbl_e;
    node_jmp -> dst = lbl_e;
    attr->list = link_ls(attr->list, ls_c);
    ir_push_b(&(attr->list), node_lbl_t);
    attr->list = link_ls(attr->list, ls_t);
    ir_push_b(&(attr->list), node_jmp);
    ir_push_b(&(attr->list), node_lbl_f);
    attr->list = link_ls(attr->list, ls_f);
    ir_push_b(&(attr->list), node_lbl_e);
  } else {
    Opr lbl_t = new_label();
    Opr lbl_f = new_label();
    GenAttr subattr = (GenAttr) {
        .list = {NULL, NULL},
        .opr = lbl_t,
        .cond = true,
        .lbl = lbl_f, };
    Irlist ls_c = gen_ir(p->stmt_if.cond, &subattr);
    Irlist ls_t = gen_ir(p->stmt_if.brnch1, NULL);
    Irnode *node_lbl_t = alloc_irnode(IR_LBL);
    Irnode *node_lbl_f = alloc_irnode(IR_LBL);
    node_lbl_t -> dst = lbl_t;
    node_lbl_f -> dst = lbl_f;
    attr->list = link_ls(attr->list, ls_c);
    ir_push_b(&(attr->list), node_lbl_t);
    attr->list = link_ls(attr->list, ls_t);
    ir_push_b(&(attr->list), node_lbl_f);
  }
}

AST_MAKE_VISIT(STMT_WHILE) {
  GenAttr *attr = arg;
  Opr lbl_cond = new_label();
  Opr lbl_stmt = new_label();
  Opr lbl_exit = new_label();
  GenAttr subattr = (GenAttr) {
      .list = {NULL, NULL},
      .opr = lbl_stmt,
      .cond = true,
      .lbl = lbl_exit, };
  Irlist ls_cond = gen_ir(p->stmt_while.cond, &subattr);
  Irlist ls_stmt = gen_ir(p->stmt_while.body, NULL);
  Irnode *node_lbl_cond = alloc_irnode(IR_LBL);
  Irnode *node_lbl_stmt = alloc_irnode(IR_LBL);
  Irnode *node_lbl_exit = alloc_irnode(IR_LBL);
  Irnode *node_jmp = alloc_irnode(IR_JMP);
  node_lbl_cond -> dst = lbl_cond;
  node_lbl_stmt -> dst = lbl_stmt;
  node_lbl_exit -> dst = lbl_exit;
  node_jmp -> dst = lbl_cond;
  ir_push_b(&(attr->list), node_lbl_cond);
  attr->list = link_ls(attr->list, ls_cond);
  ir_push_b(&(attr->list), node_lbl_stmt);
  attr->list = link_ls(attr->list, ls_stmt);
  ir_push_b(&(attr->list), node_jmp);
  ir_push_b(&(attr->list), node_lbl_exit);
}

AST_MAKE_VISIT(STMT_RET) {
  GenAttr *attr = arg;
  GenAttr subattr = (GenAttr) {
      .list = {NULL, NULL},
      .opr = new_tmpvar(),
      .cond = false, };
  attr->list = gen_ir(p->stmt_ret.exp, &subattr);
  Irnode *node_ret = alloc_irnode(IR_RET);
  node_ret->dst = subattr.opr;
  ir_push_b(&(attr->list), node_ret);
}

AST_MAKE_VISIT(STMT_COMP) {
  GenAttr *attr = arg;
  for (Astnode *q = p->stmt_comp.defs; q != NULL; q = q->nxt) {
    Irlist ls = gen_ir(q, NULL);
    attr->list = link_ls(attr->list, ls);
  }
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
  if (p->def_var.init) {
    if (p->type.kind == kINT_t) {
      GenAttr subattr = (GenAttr) {
          .list = {NULL, NULL},
          .opr = new_tmpvar(),
          .cond = false, };
      attr->list = gen_ir(p->def_var.init, &subattr);
      Irnode *node_asn = alloc_irnode(IR_ASN);
      node_asn->dst = lookup_var(p->def_var.name);
      node_asn->src1 = subattr.opr;
      ir_push_b(&(attr->list), node_asn);
    } else {
      // TODO
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

