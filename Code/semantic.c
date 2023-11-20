#include <stdio.h>
#include <string.h>
#include "semantic.h"

const char *SEM_ERROINFO[] = {
  SEM_ERR(F_STR_LIST)
};

Type *cur_ret_t = &type_err;

#define log_error(errno, lineno) \
  do { \
    ++error_cnt; \
    printf("Error type %d at Line %d: %s (in %s)\n", errno, lineno, SEM_ERROINFO[errno], __func__); \
  } while (0)

AST(AST_VISIT_FUNDEC)
VISITOR_DEF(AST, visitor)

Type* sem_check(Astnode *p) {
  if (p == NULL) return &type_err;
  AstVisitorDispatch(&visitor, p, NULL);
  return &(p->type);
}

AST_MAKE_VISIT(EXP_ID) {
  Syment *ent = symtab_lookup(p->exp_id.name);
  if (ent == NULL) {
    log_error(VAR_UNDEF, p->lineno);
    p->type = type_err;
  } else {
    p->type = *(ent->type);
  }
}

AST_MAKE_VISIT(EXP_INT) {
  p->type = type_int;
}

AST_MAKE_VISIT(EXP_FLT) {
  p->type = type_flt;
}

AST_MAKE_VISIT(EXP_UOP) {
  Type *rhs_t = sem_check(p->exp_uop.rhs);
  if (!type_isprim(rhs_t)) {
    log_error(OPR_MISMATCH, p->lineno);
    p->type = type_err;
  } else {
    p->type = *rhs_t;
  }
}

AST_MAKE_VISIT(EXP_BOP) {
  Type *lhs_t = sem_check(p->exp_bop.lhs);
  Type *rhs_t = sem_check(p->exp_bop.rhs);
  if (p->exp_bop.bop != kASSIGNOP) {
    if (!type_isprim(lhs_t) || !type_eq(lhs_t, rhs_t)) {
      log_error(OPR_MISMATCH, p->lineno);
      p->type = type_err;
    } else {
      p->type = p->exp_bop.bop >= kEQ && p->exp_bop.bop <= kGE ?
        type_int :
        *lhs_t;
    }
  } else {
    if (!type_eq(lhs_t, rhs_t)) {
      log_error(ASGN_MISMATCH, p->lineno);
      p->type = type_err;
    } else if (!is_lval(p->exp_bop.lhs)) {
      log_error(ASGN_2RVALUE, p->lineno);
      p->type = type_err;
    } else {
      p->type = *lhs_t;
    }
  }
}

AST_MAKE_VISIT(EXP_ARRAY) {
  Type *base_t = sem_check(p->exp_array.base);
  int diff = base_t->kind == kARRAY ? -base_t->array.dim : 0;
  for (Astnode *q = p->exp_array.indexs; q != NULL; q = q->nxt) {
    Type *indx_t = sem_check(q);
    ++diff;
    if (diff > 0) {
      log_error(ACCESS_NOT_ARRAY, q->lineno);
      p->type = type_err; return;
    }
    if (!type_isdisc(indx_t)) {
      log_error(ACCESS_ARRAY_NINT, q->lineno);
      p->type = type_err; return;
    }
  }
  if (diff == 0) {
    p->type = *(base_t->array.elem_t);
  } else {
    p->type.kind = kARRAY;
    p->type.array.elem_t = base_t->array.elem_t;
    p->type.array.dim = base_t->array.dim + diff;
    p->type.array.siz = base_t->array.cof[-1-diff];
    for (int i=0; i < p->type.array.dim; ++i) {
      p->type.array.len[i] = base_t->array.len[i-diff];
      p->type.array.cof[i] = base_t->array.cof[i-diff];
    }
  }
}

AST_MAKE_VISIT(EXP_FIELD) {
  Type *base_t = sem_check(p->exp_field.base);
  if (base_t->kind == kERR_t) {
    p->type = type_err;
  } else if (base_t->kind != kSTRCT) {
    log_error(ACCESS_NOT_STRCT, p->exp_field.base->lineno);
    p->type = type_err;
  } else {
    Field *field = get_field(base_t->strct.fields, p->exp_field.field);
    if (field == NULL) {
      log_error(ACCESS_FIELD_UNDEF, p->lineno);
      p->type = type_err;
    } else {
      p->type = *(field->type);
    }
  }
}

AST_MAKE_VISIT(EXP_CALL) {
  Syment *ent = symtab_lookup(p->exp_call.funct);
  if (ent == NULL) {
    log_error(FUN_UNDEF, p->lineno);
    p->type = type_err;
  } else if (ent->kind != kFUN) {
    log_error(CALL_NOT_FUNCT, p->lineno);
    p->type = type_err;
  } else {
    Type *fun_t = ent->type;
    if (fun_t->funct.paramc != p->exp_call.argc) {
      log_error(ARG_MISMATCH, p->lineno);
      p->type = type_err;
    } else {
      Param *params = fun_t->funct.params;
      for (Astnode *q = p->exp_call.args; q != NULL; q = q->nxt) {
        Type *arg_t = sem_check(q);
        if (!type_eq(arg_t, params->type)) {
          log_error(ARG_MISMATCH, q->lineno);
          p->type = type_err; return;
        }
        params = params->nxt;
      }
      p->type = *(fun_t->funct.ret_t);
    }
  }
}

AST_MAKE_VISIT(STMT_EXP) {
  sem_check(p->stmt_exp.exp);
}

AST_MAKE_VISIT(STMT_IF) {
  Type *cond_t = sem_check(p->stmt_if.cond); 
  sem_check(p->stmt_if.brnch1);
  sem_check(p->stmt_if.brnch0);
}

AST_MAKE_VISIT(STMT_WHILE) {
  Type *cond_t = sem_check(p->stmt_while.cond); 
  sem_check(p->stmt_while.body);
}

AST_MAKE_VISIT(STMT_RET) {
  Type *ret_t = sem_check(p->stmt_ret.exp); 
  if (!type_eq(ret_t, cur_ret_t))
    log_error(RET_MISMATCH, p->lineno);
}

AST_MAKE_VISIT(STMT_COMP) {
  if (arg == NULL) symtab_push_scope();
  for (Astnode *q = p->stmt_comp.defs; q != NULL; q = q->nxt) sem_check(q);
  for (Astnode *q = p->stmt_comp.stmts; q != NULL; q = q->nxt) sem_check(q);
  if (arg == NULL) symtab_pop_scope();
}

AST_MAKE_VISIT(CONSTR_SPEC) {
  if (p->constr_spec.kind == kINT_t) {
    p->type = type_int;
  } else if (p->constr_spec.kind == kFLT_t) {
    p->type = type_flt;
  } else {
    if (p->constr_spec.isdef) {
      bool ever_error = false;
      Syment *ent = symtab_insert(p->constr_spec.name);
      if (ent == NULL) {
        log_error(STRCT_REDEF, p->lineno);
        ever_error = true;
      } else {
        ent->kind = kTYP;
        ent->type = &(p->type);
      }
      p->type.kind = kSTRCT;
      strcpy(p->type.strct.name, p->constr_spec.name);
      Field *tail = NULL;
      for (Astnode *q = p->constr_spec.fields; q != NULL; q = q->nxt) {
        if (q->kind == DEF_VAR) {
          Type *elem_t = sem_check(q->def_var.spec);
          if (get_field(p->type.strct.fields, q->def_var.name) == NULL) {
            Field *f = alloc_field(&(q->type), q->def_var.name);
            if (q->def_var.dim == 0) {
              q->type = *elem_t;
            } else {
              q->type.kind = kARRAY;
              q->type.array.elem_t = elem_t;
              q->type.array.dim = q->def_var.dim;
              for (int i=0; i < q->def_var.dim; ++i)
                q->type.array.len[i] = q->def_var.len[i];
              uint32_t s = 4;
              for (int i = q->type.array.dim - 1; i >= 0; --i) {
                q->type.array.cof[i] = s;
                s *= q->type.array.len[i];
              }
              q->type.array.siz = s;
            }
            if (tail == NULL) p->type.strct.fields = f;
            else tail->nxt = f;
            tail = f;
            if (q->def_var.init != NULL) log_error(FIELD_REDEF, q->lineno);
          } else {
            log_error(FIELD_REDEF, q->lineno);
          }
        } else {
          sem_check(q);
        }
        if (ever_error) {
          free_field(p->type.strct.fields); 
          p->type = type_err;
        }
      }
    } else {
      Syment *ent = symtab_lookup(p->constr_spec.name);
      if (ent == NULL || ent->kind != kTYP) {
        log_error(STRCT_UNDEF, p->lineno);
        p->type = type_err;
      } else {
        p->type = *(ent->type);
      }
    }
  }
}

AST_MAKE_VISIT(DEF_TYP) {
  sem_check(p->def_typ.spec);
}

AST_MAKE_VISIT(DEF_VAR) {
  Type *elem_t = sem_check(p->def_var.spec);
  Syment *ent = symtab_insert(p->def_var.name);
  if (ent == NULL) {
    log_error(VAR_REDEF, p->lineno);
    p->type = type_err;
  } else {
    ent->kind = kVAR;
    ent->type = &(p->type);
    if (p->def_var.dim == 0) {
      p->type = *elem_t;
    } else {
      p->type.kind = kARRAY;
      p->type.array.elem_t = elem_t;
      p->type.array.dim = p->def_var.dim;
      for (int i=0; i < p->def_var.dim; ++i)
        p->type.array.len[i] = p->def_var.len[i];
      uint32_t s = 4;
      for (int i = p->type.array.dim - 1; i >= 0; --i) {
        p->type.array.cof[i] = s;
        s *= p->type.array.len[i];
      }
      p->type.array.siz = s;
    }
    if (p->def_var.init != NULL) {
      Type *rhs_t = sem_check(p->def_var.init);
      if (!type_eq(&(p->type), rhs_t))
        log_error(ASGN_MISMATCH, p->lineno);
    }
  }
}

AST_MAKE_VISIT(DEF_FUN) {
  Type *ret_t = sem_check(p->def_fun.spec);
  Syment *ent = symtab_insert(p->def_fun.name);
  if (ent == NULL) {
    log_error(FUN_REDEF, p->lineno);
    p->type = type_err; 
  } else {
    ent->kind = kFUN;
    ent->type = &(p->type);
    p->type.kind = kFUNCT;
    p->type.funct.ret_t = ret_t;
    p->type.funct.params = NULL;
    p->type.funct.paramc = p->def_fun.paramc;
  }
  symtab_push_scope();
  Param *tail = NULL;
  for (Astnode *q = p->def_fun.params; q != NULL; q = q->nxt) {
    Type *param_t = sem_check(q);
    if (p->type.kind == kFUNCT) {
      Param *f = alloc_field(param_t, q->def_var.name);
      if (tail == NULL) p->type.funct.params = f;
      else tail->nxt = f;
      tail = f;
    }
  }
  Type *save_ret_t = cur_ret_t;
  cur_ret_t = ret_t;
  Visit_STMT_COMP(p->def_fun.body, p);
  cur_ret_t = save_ret_t;
  symtab_pop_scope();
}

AST_MAKE_VISIT(LS_PROG) {
  symtab_push_scope();
  Syment *entr = symtab_insert("read");
  Syment *entw = symtab_insert("write");
  entr->kind = kFUN; entr->type = &type_read;
  entw->kind = kFUN; entw->type = &type_write;
  for (Astnode *q = p->ls_prog.defs; q != NULL; q = q->nxt) sem_check(q);
  symtab_pop_scope();
}

