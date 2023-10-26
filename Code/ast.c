#include <stdlib.h>
#include <string.h>
#include "ast.h"

const char *AST_KIND_NAME[] = {
  AST(F_STR_LIST)
};

int anonymous_struct_cnt = 0;

Astnode* ast_init(TYP_KIND kind, int lineno) {
  Astnode *p = (Astnode*) malloc(sizeof(Astnode));
  p->kind = kind;
  p->lineno = lineno;
  p->nxt = NULL;
  return p;
}

Astnode* ast_typeref(Astnode *def) {
 Astnode *ref = (Astnode*) malloc(sizeof(Astnode));
 *ref = *def;
 if (ref->constr_spec.kind == kSTRCT && ref->constr_spec.isdef) {
   ref->constr_spec.isdef = false;
   ref->constr_spec.fields = NULL;
 }
 return ref;
}

Astnode* ast_args(Cstnode *p, int *cnt) {
  Astnode *q = ast_exp(p->chd[0]);
  if (p->chd_num == 3) {
    q->nxt = ast_args(p->chd[2], cnt); 
    *cnt += 1;
  } else {
    q->nxt = NULL;
    *cnt = 1;
  }
  return q;
}

Astnode* ast_stmts(Cstnode *p) {
  if (p == NULL) return NULL;
  Astnode *q = ast_stmt(p->chd[0]);
  q->nxt = p->chd_num == 2 ? ast_stmts(p->chd[1]) : NULL;
  return q;
}

Astnode *ast_defs(Cstnode *p) {
  if (p == NULL) return NULL;
  Astnode *head = ast_def(p->chd[0]);
  if (head == NULL) {
    return p->chd_num == 2 ? ast_defs(p->chd[1]) : NULL;
  } else {
    Astnode *tail = head;
    while (tail->nxt != NULL) tail = tail->nxt;
    tail->nxt = p->chd_num == 2 ? ast_defs(p->chd[1]) : NULL;
    return head;
  }
}

Astnode *ast_decs(Cstnode *p, Astnode *spec) {
  Astnode *q = ast_dec(p->chd[0], spec);
  q->nxt = p->chd_num == 3 ? ast_decs(p->chd[2], spec) : NULL;
  return q;
}

Astnode *ast_params(Cstnode *p, int *cnt) {
  Astnode *q = ast_param(p->chd[0]);
  if (p->chd_num == 3) {
    q->nxt = ast_params(p->chd[2], cnt);
    *cnt += 1;
  } else {
    q->nxt = NULL;
    *cnt = 1;
  }
  return q;
}

Astnode* ast_exp(Cstnode *p) {
  // PAREN
  if (p->chd[0]->type == kLP) return ast_exp(p->chd[1]);
  // INT FLT ID
  if (p->chd_num == 1) {
    if (p->chd[0]->type == kINT) {
      Astnode *q = ast_init(EXP_INT, p->lineno);
      q->exp_int.val = p->chd[0]->int_val;
      return q;
    } else if (p->chd[0]->type == kFLOAT) {
      Astnode *q = ast_init(EXP_FLT, p->lineno);
      q->exp_flt.val = p->chd[0]->float_val;
      return q;
    } else if (p->chd[0]->type == kID) {
      Astnode *q = ast_init(EXP_ID, p->lineno);
      strcpy(q->exp_id.name, p->chd[0]->str_val);
      return q;
    }
    return NULL;
  }
  // CALL
  if (p->chd[1]->type == kLP) {
    Astnode *q = ast_init(EXP_CALL, p->lineno);
    strcpy(q->exp_call.funct, p->chd[0]->str_val);
    if (p->chd[2]->type == kRP) {
      q->exp_call.args = NULL;
      q->exp_call.argc = 0;
    } else {
      q->exp_call.args = ast_args(p->chd[2], &q->exp_call.argc);
    }
    return q;
  }
  // ARRAY
  if (p->chd[1]->type == kLB) {
    Astnode *q = ast_init(EXP_ARRAY, p->lineno);
    q->exp_array.indexs = NULL;
    q->exp_array.indexc = 0;
    while (p->chd_num == 4 && p->chd[1]->type == kLB) {
      q->exp_array.indexc += 1;
      Astnode *index = ast_exp(p->chd[2]);
      index->nxt = q->exp_array.indexs;
      q->exp_array.indexs = index;
      p = p->chd[0];
    }
    q->exp_array.base = ast_exp(p);
    return q;
  }
  // FIELD
  if (p->chd[1]->type == kDOT) {
    Astnode *q = ast_init(EXP_FIELD, p->lineno);
    q->exp_field.base = ast_exp(p->chd[0]);
    strcpy(q->exp_field.field, p->chd[2]->str_val);
    return q;
  }
  // UOP
  if (p->chd_num == 2) {
    Astnode *q = ast_init(EXP_UOP, p->lineno);
    q->exp_uop.uop = p->chd[0]->type;
    q->exp_uop.rhs = ast_exp(p->chd[1]);
    return q;
  }
  // BOP
  if (p->chd_num == 3) {
    Astnode *q = ast_init(EXP_BOP, p->lineno);
    q->exp_bop.lhs = ast_exp(p->chd[0]);
    q->exp_bop.bop = p->chd[1]->type;
    q->exp_bop.rhs = ast_exp(p->chd[2]);
    return q;
  }
  return NULL;
}

Astnode* ast_stmt(Cstnode *p) {
  if (!strcmp(p->chd[0]->name, "CompSt") || !strcmp(p->name, "CompSt")) {
    Astnode *q = ast_init(STMT_COMP, p->lineno);
    q->stmt_comp.defs = NULL;
    q->stmt_comp.stmts = NULL;
    if (!strcmp(p->chd[0]->name, "CompSt")) p = p->chd[0];
    for (int i=1; i+1<p->chd_num; ++i) {
      if (!strcmp(p->chd[i]->name, "DefList")) {
        q->stmt_comp.defs = ast_defs(p->chd[i]);
      } else {
        q->stmt_comp.stmts = ast_stmts(p->chd[i]);
      }
    }
    return q;
  }
  if (p->chd[1]->type == kSEMI) {
    Astnode *q = ast_init(STMT_EXP, p->lineno);
    q->stmt_exp.exp = ast_exp(p->chd[0]);
    return q;
  }
  if (p->chd[0]->type == kRETURN) {
    Astnode *q = ast_init(STMT_RET, p->lineno);
    q->stmt_ret.exp = ast_exp(p->chd[1]);
    return q;
  }
  if (p->chd[0]->type == kIF) {
    Astnode *q = ast_init(STMT_IF, p->lineno);
    q->stmt_if.cond = ast_exp(p->chd[2]);
    q->stmt_if.brnch1 = ast_stmt(p->chd[4]);
    q->stmt_if.brnch0 = p->chd_num == 7 ? ast_stmt(p->chd[6]) : NULL;
    return q;
  }
  if (p->chd[0]->type == kWHILE) {
    Astnode *q = ast_init(STMT_WHILE, p->lineno);
    q->stmt_while.cond = ast_exp(p->chd[2]);
    q->stmt_while.body = ast_stmt(p->chd[4]);
    return q;
  }
  return NULL;
}

Astnode* ast_spec(Cstnode *p) {
  Astnode *q = ast_init(CONSTR_SPEC, p->lineno);
  if (p->chd[0]->type == kTYPE) {
    q->constr_spec.kind = (!strcmp(p->chd[0]->str_val, "int")) ?
                          kINT_t : kFLT_t;
  } else {
    p = p->chd[0];
    q->constr_spec.kind = kSTRCT;
    if (p->chd_num == 2) {
      strcpy(q->constr_spec.name, p->chd[1]->chd[0]->str_val);
      q->constr_spec.isdef = false;
      q->constr_spec.fields = NULL;
    } else {
      if (!strcmp(p->chd[1]->name, "OptTag")) {
        strcpy(q->constr_spec.name, p->chd[1]->chd[0]->str_val);
      } else {
        symstr str;
        sprintf(str, "%d", anonymous_struct_cnt);
        strcpy(q->constr_spec.name, str);
        ++anonymous_struct_cnt;
      }
      q->constr_spec.isdef = true;
      q->constr_spec.fields =
          (!strcmp(p->chd[p->chd_num - 2]->name, "DefList")) ?
          ast_defs(p->chd[p->chd_num - 2]) : NULL;
    }
  }
  return q;
}

Astnode* ast_def(Cstnode *p) {
  Astnode *spec = ast_spec(p->chd[0]);
  if (p->chd_num == 2) {
    if (spec->constr_spec.kind == kSTRCT && spec->constr_spec.isdef) {
      Astnode *q = ast_init(DEF_TYP, p->lineno);
      q->def_typ.spec = spec;
      return q;
    } else {
      return NULL;
    }
  }
  if (p->chd[2]->type == kSEMI) {
    Astnode *typ_def = NULL;
    bool is_constr_struct = spec->constr_spec.kind == kSTRCT && spec->constr_spec.isdef;
    if (is_constr_struct) {
      Astnode *q = ast_init(DEF_TYP, p->lineno);
      q->def_typ.spec = spec;
      typ_def = q;
    }
    Astnode *var_defs = ast_decs(p->chd[1], spec);
    if (!is_constr_struct) free_ast(spec);
    if (typ_def == NULL) {
      return var_defs;
    } else {
      typ_def->nxt = var_defs;
      return typ_def;
    }
  }
  Astnode *q = ast_init(DEF_FUN, p->lineno);
  q->def_fun.spec = spec;
  q->def_fun.body = ast_stmt(p->chd[2]);
  p = p->chd[1];
  strcpy(q->def_fun.name, p->chd[0]->str_val);
  q->def_fun.params = p->chd_num == 4 ?
      ast_params(p->chd[2], &q->def_fun.paramc) : NULL;
  return q;
}

Astnode* ast_dec(Cstnode *p, Astnode *spec) {
  Astnode *q = ast_init(DEF_VAR, p->lineno);
  q->def_var.spec = ast_typeref(spec);
  q->def_var.init = (!strcmp(p->name, "Dec") && p->chd_num == 3) ?
                    ast_exp(p->chd[2]) : NULL;
  if (!strcmp(p->name, "Dec")) p = p->chd[0];
  q->def_var.dim = 0;
  while (p->chd_num == 4) {
    q->def_var.len[q->def_var.dim] = p->chd[2]->int_val;
    q->def_var.dim += 1;
    p = p->chd[0];
  }
  strcpy(q->def_var.name, p->chd[0]->str_val);
  for (int i=0, j = q->def_var.dim-1; i<j; ++i, --j) {
    int t = q->def_var.len[i];
    q->def_var.len[i] = q->def_var.len[j];
    q->def_var.len[j] = t;
  }
  return q;
}

Astnode* ast_param(Cstnode *p) {
  Astnode *q = ast_init(DEF_VAR, p->lineno);
  q->def_var.spec = ast_spec(p->chd[0]);
  q->def_var.init = NULL;
  q->def_var.dim = 0;
  p = p->chd[1];
  while (p->chd_num == 4) {
    q->def_var.len[q->def_var.dim] = p->chd[2]->int_val;
    q->def_var.dim += 1;
    p = p->chd[0];
  }
  strcpy(q->def_var.name, p->chd[0]->str_val);
  return q;
}

Astnode* ast_prog(Cstnode *p) {
  Astnode *q = ast_init(LS_PROG, p->lineno);
  q->ls_prog.defs = ast_defs(p->chd[0]);
  return q;
}

void print_ast(Astnode *p, int indent) {
  if (p == NULL) return;
  for (int i=0; i<indent; ++i) putchar(' ');
  printf("%2d %s", p->lineno, AST_KIND_NAME[p->kind]);
  switch (p->kind) {
    // EXPRESSION
    case EXP_ID: printf(": %s\n", p->exp_id.name); break;
    case EXP_INT: printf(": %d\n", p->exp_int.val); break;
    case EXP_FLT: printf(": %.6f\n", p->exp_flt.val); break;
    case EXP_UOP:
      printf(": %s\n", token_name[p->exp_uop.uop]);
      print_ast(p->exp_uop.rhs, indent+2);
      break;
    case EXP_BOP:
      printf(": %s\n", token_name[p->exp_bop.bop]);
      print_ast(p->exp_bop.lhs, indent+2);
      print_ast(p->exp_bop.rhs, indent+2);
      break;
    case EXP_ARRAY:
      printf(": (%d)\n", p->exp_array.indexc);
      print_ast(p->exp_array.base, indent+2);
      print_ast(p->exp_array.indexs, indent+2);
      break;
    case EXP_FIELD:
      printf(": %s\n", p->exp_field.field);
      print_ast(p->exp_field.base, indent+2);
      break;
    case EXP_CALL:
      printf(": %s (%d)\n", p->exp_call.funct, p->exp_call.argc);
      print_ast(p->exp_call.args, indent+2);
      break;
    // STATEMENT
    case STMT_EXP:
      putchar('\n');
      print_ast(p->stmt_exp.exp, indent+2);
      break;
    case STMT_IF:
      putchar('\n');
      print_ast(p->stmt_if.cond, indent+2);
      print_ast(p->stmt_if.brnch1, indent+2);
      print_ast(p->stmt_if.brnch0, indent+2);
      break;
    case STMT_WHILE:
      putchar('\n');
      print_ast(p->stmt_while.cond, indent+2);
      print_ast(p->stmt_while.body, indent+2);
      break;
    case STMT_RET:
      putchar('\n');
      print_ast(p->stmt_ret.exp, indent+2);
      break;
    case STMT_COMP:
      putchar('\n');
      print_ast(p->stmt_comp.defs, indent+2);
      print_ast(p->stmt_comp.stmts, indent+2);
      break;
    // DEFINITION
    case CONSTR_SPEC:
      if (p->constr_spec.kind == kINT_t) {
        printf(": INT\n");
      } else if (p->constr_spec.kind == kFLT_t) {
        printf(": FLT\n");
      } else {
        printf(": STRCT %s\n", p->constr_spec.name);
        if (p->constr_spec.isdef) {
          print_ast(p->constr_spec.fields, indent+2);
        }
      }
      break;
    case DEF_TYP:
      putchar('\n');
      print_ast(p->def_typ.spec, indent+2);
      break;
    case DEF_VAR:
      printf(" %s (%d)", p->def_var.name, p->def_var.dim);
      for (int i=0; i<p->def_var.dim; ++i) printf(" %d", p->def_var.len[i]);
      putchar('\n');
      print_ast(p->def_var.spec, indent+2);
      print_ast(p->def_var.init, indent+2);
      break;
    case DEF_FUN:
      printf(": %s (%d)\n", p->def_fun.name, p->def_fun.paramc);
      print_ast(p->def_fun.spec, indent+2);
      print_ast(p->def_fun.params, indent+2);
      print_ast(p->def_fun.body, indent+2);
      break;
    case LS_PROG:
      putchar('\n');
      print_ast(p->ls_prog.defs, indent+2);
      break;
    default: return;
  }
  print_ast(p->nxt, indent);
}

void free_ast(Astnode *p) {
  if (p == NULL) return;
  if (p->nxt != NULL) free_ast(p->nxt);
  // EXPRESSION
  if (p->kind == EXP_UOP) free_ast(p->exp_uop.rhs);
  if (p->kind == EXP_BOP) free_ast(p->exp_bop.lhs), free_ast(p->exp_bop.rhs);
  if (p->kind == EXP_ARRAY) free_ast(p->exp_array.base), free_ast(p->exp_array.indexs);
  if (p->kind == EXP_FIELD) free_ast(p->exp_field.base);
  if (p->kind == EXP_CALL) free_ast(p->exp_call.args);
  // STATEMENT
  if (p->kind == STMT_EXP) free_ast(p->stmt_exp.exp);
  if (p->kind == STMT_IF) free_ast(p->stmt_if.cond), free_ast(p->stmt_if.brnch1), free_ast(p->stmt_if.brnch0);
  if (p->kind == STMT_WHILE) free_ast(p->stmt_while.cond), free_ast(p->stmt_while.body);
  if (p->kind == STMT_RET) free_ast(p->stmt_ret.exp);
  if (p->kind == STMT_COMP) free_ast(p->stmt_comp.defs), free_ast(p->stmt_comp.stmts);
  // DEFINITION
  if (p->kind == CONSTR_SPEC && p->constr_spec.kind == kSTRCT && p->constr_spec.isdef) {
    free_ast(p->constr_spec.fields);
    free_field(p->type.strct.fields);
  }
  if (p->kind == DEF_TYP) free_ast(p->def_fun.spec);
  if (p->kind == DEF_VAR) free_ast(p->def_var.spec), free_ast(p->def_var.init);
  if (p->kind == DEF_FUN) {
    free_ast(p->def_fun.spec), free_ast(p->def_fun.params), free_ast(p->def_fun.body);
    if (p->type.kind == kFUNCT) free_field(p->type.funct.params);
  }
  if (p->kind == LS_PROG) free_ast(p->ls_prog.defs);
  free(p);
}

void AstVisitorDispatch(ASTVisitor *visitor, Astnode *p, void *arg) {
  if (p == NULL) return;
  switch (p->kind) {
    AST(F_DISPATCH)
  }
}

bool is_lval(Astnode *p) {
  if (p == NULL) return false;
  if (p->kind == EXP_ID) return true;
  if (p->kind == EXP_ARRAY) return true;
  if (p->kind == EXP_FIELD) return true;
  return false;
}

