#include <stdio.h>
#include <string.h>
#include "symtab.h"
#include "ir.h"

int cnt_var = 0, cnt_tmp = 0, cnt_lbl = 0;

static Opr make_litval(int litval) {
  return (Opr) { .kind = OPR_LIT, .litval = litval,
                 .ref = false, .deref = false, };
}
static Opr lookup_var(const char* name) {
  Syment *ent = symtab_lookup(name);
  if (ent->varno == 0) ent->varno = ++cnt_var;
  return (Opr) { .kind = OPR_VAR, .tmpno = ent->varno,
                 .ref = ent->type->kind == kARRAY && !ent->ref,
                 .deref = false, };
}
static Opr new_tmpvar() {
  return (Opr) { .kind = OPR_TMP, .tmpno = ++cnt_tmp,
                 .ref = false, .deref = false, };
}
static Opr new_label() {
  return (Opr) { .kind = OPR_LBL, .lblno = ++cnt_lbl,
                 .ref = false, .deref = false, };
}

typedef struct {
  Irlist list;
  Opr opr;
  bool cond;
  Opr lbl;
} GenAttr;

static GenAttr attr_cond(Opr lbl_t, Opr lbl_f) {
  return (GenAttr) { .list = {NULL, NULL}, .opr = lbl_t,
                     .cond = true, .lbl = lbl_f, };
}
static GenAttr attr_tmpvar() {
  return (GenAttr) { .list = {NULL, NULL}, .opr = new_tmpvar(),
                     .cond = false, };
}

AST(AST_VISIT_FUNDEC)
VISITOR_DEF(AST, visitor)

Irlist gen_ir(Astnode *p, void *arg) {
  if (p == NULL) return (Irlist) {NULL, NULL};
  if (arg == NULL) {
    GenAttr attr = (GenAttr) {
        .list = {NULL, NULL}, .opr = {OPR_TMP, 0, false, false},
        .cond = false, };
    arg = &attr;
  }
  AstVisitorDispatch(&visitor, p, arg);
  return ((GenAttr*) arg)->list;
}

static void gen_cond_ari(Irlist *ls, Opr val, Opr opr, Opr lbl) {
  ir_push_b(ls, alloc_ir_dsso(IR_JCN, opr, val, make_litval(0), kNEQ));
  ir_push_b(ls, alloc_ir_d(IR_JMP, lbl));
}

static void gen_exp_bool_pre(Irlist *ls, Opr opr) {
  ir_push_b(ls, alloc_ir_ds(IR_ASN, opr, make_litval(0)));
}

static void gen_exp_bool_post(Irlist *ls, Opr opr, Opr lbl_t, Opr lbl_f) {
  ir_push_b(ls, alloc_ir_d(IR_LBL, lbl_t));
  ir_push_b(ls, alloc_ir_d(IR_LBL, lbl_f));
  ir_push_b(ls, alloc_ir_ds(IR_ASN, opr, make_litval(1)));
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
    GenAttr subattr = attr_tmpvar();
    attr->list = gen_ir(p->exp_uop.rhs, &subattr);
    Irnode *node_neg = alloc_ir_dsso(IR_ARI,
        attr->cond ? new_tmpvar() : attr->opr,
        make_litval(0), subattr.opr, kMINUS);
    ir_push_b(&(attr->list), node_neg);
    if (attr->cond)
      gen_cond_ari(&(attr->list), node_neg->dst, attr->opr, attr->lbl);
  } else {
    Opr save = attr->opr;
    if (!(attr->cond)) {
      gen_exp_bool_pre(&(attr->list), save);
      attr->opr = new_label(), attr->lbl = new_label();
    }
    GenAttr subattr = attr_cond(attr->lbl, attr->opr);
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
      GenAttr subattr = attr_tmpvar();
      attr->list = gen_ir(p->exp_bop.rhs, &subattr);
      Opr opr_sym = lookup_var(p->exp_bop.lhs->exp_id.name);
      ir_push_b(&(attr->list), alloc_ir_ds(IR_ASN, opr_sym, subattr.opr));
      if (attr->cond) {
        gen_cond_ari(&(attr->list), opr_sym, attr->opr, attr->lbl);
      } else {
        if (attr->opr.litval != 0) --cnt_tmp;
        attr->opr = opr_sym;
      }
    } else {
      // TODO
    }
  } else if (p->exp_bop.bop >= kPLUS && p->exp_bop.bop <= kDIV) {
    GenAttr subattr_l = attr_tmpvar();
    attr->list = link_ls(attr->list, gen_ir(p->exp_bop.lhs, &subattr_l));
    GenAttr subattr_r = attr_tmpvar();
    attr->list = link_ls(attr->list, gen_ir(p->exp_bop.rhs, &subattr_r));
    Irnode *node_ari = alloc_ir_dsso(IR_ARI,
        attr->cond ? new_tmpvar() : attr->opr,
        subattr_l.opr, subattr_r.opr, p->exp_bop.bop);
    ir_push_b(&(attr->list), node_ari);
    if (attr->cond)
      gen_cond_ari(&(attr->list), node_ari->dst, attr->opr, attr->lbl);
  } else if (p->exp_bop.bop >= kAND && p->exp_bop.bop <= kOR) {
    Opr save = attr->opr;
    if (!(attr->cond)) {
      gen_exp_bool_pre(&(attr->list), save);
      attr->opr = new_label(), attr->lbl = new_label();
    }
    Opr lbl_m = new_label();
    GenAttr subattr_l = attr_cond(
        p->exp_bop.bop == kAND ? lbl_m : attr->opr,
        p->exp_bop.bop == kAND ? attr->lbl : lbl_m);
    GenAttr subattr_r = attr_cond(attr->opr, attr->lbl);
    attr->list = link_ls(attr->list, gen_ir(p->exp_bop.lhs, &subattr_l));
    ir_push_b(&(attr->list), alloc_ir_d(IR_LBL, lbl_m));
    attr->list = link_ls(attr->list, gen_ir(p->exp_bop.rhs, &subattr_r));
    if (!(attr->cond)) {
      gen_exp_bool_post(&(attr->list), save, attr->opr, attr->lbl);
      attr->opr = save;
    }
  } else if (p->exp_bop.bop >= kEQ && p->exp_bop.bop <= kGE) {
    Opr save = attr->opr;
    if (!(attr->cond)) {
      gen_exp_bool_pre(&(attr->list), save);
      attr->opr = new_label(), attr->lbl = new_label();
    }
    GenAttr subattr_l = attr_tmpvar();
    attr->list = link_ls(attr->list, gen_ir(p->exp_bop.lhs, &subattr_l));
    GenAttr subattr_r = attr_tmpvar();
    attr->list = link_ls(attr->list, gen_ir(p->exp_bop.rhs, &subattr_r));
    ir_push_b(&(attr->list), alloc_ir_dsso(IR_JCN, attr->opr,
        subattr_l.opr, subattr_r.opr, p->exp_bop.bop));
    ir_push_b(&(attr->list), alloc_ir_d(IR_JMP, attr->lbl));
    if (!(attr->cond)) {
      gen_exp_bool_post(&(attr->list), save, attr->opr, attr->lbl);
      attr->opr = save;
    }
  }
}

AST_MAKE_VISIT(EXP_ARRAY) {
  GenAttr *attr = arg;
  GenAttr subattr = attr_tmpvar();
  attr->list = gen_ir(p->exp_array.base, &subattr);
  Opr opr_adr = new_tmpvar(), opr_off = new_tmpvar();
  ir_push_b(&(attr->list), alloc_ir_ds(IR_ASN, opr_adr, subattr.opr));
  int i = 0;
  Type *typ = &(p->exp_array.base->type);
  for (Astnode *q = p->exp_array.indexs; q != NULL; q = q->nxt) {
    GenAttr subattr_indx = attr_tmpvar();
    attr->list = link_ls(attr->list, gen_ir(q, &subattr_indx));
    ir_push_b(&(attr->list), alloc_ir_dsso(IR_ARI, opr_off,
        make_litval(typ->array.cof[i]), subattr_indx.opr, kSTAR));
    ir_push_b(&(attr->list), alloc_ir_dsso(IR_ARI, opr_adr,
        opr_adr, opr_off, kPLUS));
    ++i;
  }
  if (p->type.kind == kINT_t) opr_adr.deref = true;
  if (attr->cond) {
    gen_cond_ari(&(attr->list), opr_adr, attr->opr, attr->lbl);
  } else {
    if (attr->opr.litval != 0) --cnt_tmp;
    attr->opr = opr_adr;
  }
}

AST_MAKE_VISIT(EXP_FIELD) {  // NOT SUPPORTED
  ++error_cnt;
  printf("Cannot translate: float.\n");
}

AST_MAKE_VISIT(EXP_CALL) {
  GenAttr *attr = arg;
  if (!strcmp(p->exp_call.funct, "read")) {
    Irnode *node_red = alloc_ir_d(IR_RED,
        attr->cond ? new_tmpvar() : attr->opr);
    ir_push_b(&(attr->list), node_red);
    if (attr->cond)
      gen_cond_ari(&(attr->list), node_red->dst, attr->opr, attr->lbl);
  } else if (!strcmp(p->exp_call.funct, "write")) {
    GenAttr subattr = attr_tmpvar();
    attr->list = gen_ir(p->exp_call.args, &subattr);
    ir_push_b(&(attr->list), alloc_ir_d(IR_RIT, subattr.opr));
    ir_push_b(&(attr->list), alloc_ir_ds(IR_ASN,
        attr->cond ? new_tmpvar() : attr->opr, make_litval(0)));
    if (attr->cond)
      ir_push_b(&(attr->list), alloc_ir_d(IR_JMP, attr->lbl));
  } else {
    Irlist ls_init = (Irlist) {NULL, NULL};
    Irnode *node_cal = alloc_ir_df(IR_CAL,
        attr->cond ? new_tmpvar() : attr->opr, p->exp_call.funct);
    ir_push_f(&(attr->list), node_cal);
    for (Astnode *q = p->exp_call.args; q != NULL; q = q->nxt) {
      GenAttr subattr = attr_tmpvar();
      ls_init = link_ls(ls_init, gen_ir(q, &subattr));
      ir_push_f(&(attr->list), alloc_ir_d(IR_ARG, subattr.opr));
    }
    attr->list = link_ls(ls_init, attr->list);
    if (attr->cond)
      gen_cond_ari(&(attr->list), node_cal->dst, attr->opr, attr->lbl);
  }
}

AST_MAKE_VISIT(STMT_EXP) {
  GenAttr *attr = arg;
  attr->list = gen_ir(p->stmt_exp.exp, NULL);
}

AST_MAKE_VISIT(STMT_IF) {
  GenAttr *attr = arg;
  Opr lbl_t = new_label(), lbl_f = new_label();
  GenAttr subattr = attr_cond(lbl_t, lbl_f);
  attr->list = link_ls(attr->list, gen_ir(p->stmt_if.cond, &subattr));
  ir_push_b(&(attr->list), alloc_ir_d(IR_LBL, lbl_t));
  attr->list = link_ls(attr->list, gen_ir(p->stmt_if.brnch1, NULL));
  if (p->stmt_if.brnch0 != NULL) {
    Opr lbl_e = new_label();
    ir_push_b(&(attr->list), alloc_ir_d(IR_JMP, lbl_e));
    ir_push_b(&(attr->list), alloc_ir_d(IR_LBL, lbl_f));
    attr->list = link_ls(attr->list, gen_ir(p->stmt_if.brnch0, NULL));
    ir_push_b(&(attr->list), alloc_ir_d(IR_LBL, lbl_e));
  } else {
    ir_push_b(&(attr->list), alloc_ir_d(IR_LBL, lbl_f));
  }
}

AST_MAKE_VISIT(STMT_WHILE) {
  GenAttr *attr = arg;
  Opr lbl_cond = new_label(), lbl_stmt = new_label(), lbl_exit = new_label();
  GenAttr subattr = attr_cond(lbl_stmt, lbl_exit);
  ir_push_b(&(attr->list), alloc_ir_d(IR_LBL, lbl_cond));
  attr->list = link_ls(attr->list, gen_ir(p->stmt_while.cond, &subattr));
  ir_push_b(&(attr->list), alloc_ir_d(IR_LBL, lbl_stmt));
  attr->list = link_ls(attr->list, gen_ir(p->stmt_while.body, NULL));
  ir_push_b(&(attr->list), alloc_ir_d(IR_JMP, lbl_cond));
  ir_push_b(&(attr->list), alloc_ir_d(IR_LBL, lbl_exit));
}

AST_MAKE_VISIT(STMT_RET) {
  GenAttr *attr = arg;
  GenAttr subattr = attr_tmpvar();
  attr->list = gen_ir(p->stmt_ret.exp, &subattr);
  ir_push_b(&(attr->list), alloc_ir_d(IR_RET, subattr.opr));
}

AST_MAKE_VISIT(STMT_COMP) {
  GenAttr *attr = arg;
  for (Astnode *q = p->stmt_comp.defs; q != NULL; q = q->nxt)
    attr->list = link_ls(attr->list, gen_ir(q, NULL));
  for (Astnode *q = p->stmt_comp.stmts; q != NULL; q = q->nxt)
    attr->list = link_ls(attr->list, gen_ir(q, NULL));
}

AST_MAKE_VISIT(CONSTR_SPEC) {}

AST_MAKE_VISIT(DEF_TYP) {}

AST_MAKE_VISIT(DEF_VAR) {
  GenAttr *attr = arg;
  Syment *ent = symtab_insert(p->def_var.name);
  ent->kind = kVAR;
  ent->type = &(p->type);
  if (attr->cond) {  // PARAMETER
    ent->varno = ++cnt_var;
    ent->ref = true;
  } else {
    if (ent->type->kind == kARRAY) {  // DEC
      ent->varno = ++cnt_var;
      ent->ref = false;
      Irnode *node_dec = alloc_ir_ds(IR_DEC,
          lookup_var(p->def_var.name), make_litval(p->type.array.siz));
      node_dec->dst.ref = false;
      ir_push_b(&(attr->list), node_dec);
    } else {  // LAZY_LABELLING
      ent->varno = 0;
    }
  }
  if (p->def_var.init) {
    if (p->type.kind == kINT_t) {
      GenAttr subattr = attr_tmpvar();
      attr->list = gen_ir(p->def_var.init, &subattr);
      ir_push_b(&(attr->list), alloc_ir_ds(IR_ASN,
          lookup_var(p->def_var.name), subattr.opr));
    } else {
      // TODO
    }
  }
}

AST_MAKE_VISIT(DEF_FUN) {
  GenAttr *attr = arg;
  ir_push_b(&(attr->list), alloc_ir_f(IR_FUN, p->def_fun.name));
  for (Astnode *q = p->def_fun.params; q != NULL; q = q->nxt) {
    gen_ir(q, &((GenAttr) { .cond = true, }));
    ir_push_b(&(attr->list), alloc_ir_d(IR_PAR, lookup_var(q->def_var.name)));
  }
  attr->list = link_ls(attr->list, gen_ir(p->def_fun.body, NULL));
}

AST_MAKE_VISIT(LS_PROG) {
  GenAttr *attr = arg;
  symtab_push_scope();
  for (Astnode *q = p->ls_prog.defs; q != NULL; q = q->nxt)
    attr->list = link_ls(attr->list, gen_ir(q, NULL));
  symtab_pop_scope();
}

