#ifndef CMMC_AST_H
#define CMMC_AST_H

#include <stdbool.h>
#include <stdint.h>
#include "util.h"
#include "cst.h"
#include "type.h"

#define AST(F) \
    /* EXPRESSION */ \
    F(EXP_ID) \
    F(EXP_INT) \
    F(EXP_FLT) \
    F(EXP_UOP) \
    F(EXP_BOP) \
    F(EXP_ARRAY) \
    F(EXP_FIELD) \
    F(EXP_CALL) \
    /* STATEMENT */ \
    F(STMT_EXP) \
    F(STMT_IF) \
    F(STMT_WHILE) \
    F(STMT_RET) \
    F(STMT_COMP) \
    /* DEFINITION */ \
    F(CONSTR_SPEC) \
    F(DEF_TYP) \
    F(DEF_VAR) \
    F(DEF_FUN) \
    F(LS_PROG)

typedef enum {
  AST(F_LIST)
} AST_KIND;

typedef struct Astnode {
  AST_KIND kind;
  int lineno;
  Type type;
  struct Astnode *nxt;
  union {
    // EXPRESSION
    struct { symstr name; } exp_id;
    struct { int val; } exp_int;
    struct { float val; } exp_flt;
    struct { Token uop; struct Astnode *rhs; } exp_uop;
    struct { Token bop; struct Astnode *lhs, *rhs; } exp_bop;
    struct { struct Astnode *base, *indexs; int indexc; } exp_array;
    struct { struct Astnode *base; symstr field; } exp_field;
    struct { symstr funct; struct Astnode *args; int argc; } exp_call;
    // STATEMENT
    struct { struct Astnode *exp; } stmt_exp;
    struct { struct Astnode *cond, *brnch1, *brnch0; } stmt_if;
    struct { struct Astnode *cond, *body; } stmt_while;
    struct { struct Astnode *exp; } stmt_ret;
    struct { struct Astnode *defs, *stmts; } stmt_comp;
    // DEFINITION
    struct {
      TYP_KIND kind;
      symstr name;
      bool isdef;
      struct Astnode *fields;
    } constr_spec;
    struct {
      struct Astnode *spec;
    } def_typ;
    struct {
      struct Astnode *spec;
      symstr name;
      uint32_t len[MAX_ARRAY_DIMSN_NUM], dim;
      struct Astnode *init;
    } def_var;
    struct {
      symstr name;
      struct Astnode *spec;
      struct Astnode *params; 
      struct Astnode *body;
      int paramc;
    } def_fun;
    struct { struct Astnode *defs; } ls_prog;
  };
} Astnode;

typedef void (*AstVisitT)(Astnode*, void*);
#define AST_VISIT_FUNPTR(KIND) AstVisitT VisitPtr_##KIND;
#define AST_MAKE_VISIT(KIND) static void Visit_##KIND(Astnode *p, void *arg)
#define AST_VISIT_FUNDEC(KIND) AST_MAKE_VISIT(KIND);

typedef struct ASTVisitor {
  AST(AST_VISIT_FUNPTR)
} ASTVisitor;

Astnode* ast_exp(Cstnode*);
Astnode* ast_args(Cstnode*, int*);
Astnode* ast_stmt(Cstnode*);
Astnode* ast_spec(Cstnode*);
Astnode* ast_def(Cstnode*);
Astnode* ast_defs(Cstnode*);
Astnode* ast_dec(Cstnode*, Astnode*);
Astnode* ast_decs(Cstnode*, Astnode*);
Astnode* ast_param(Cstnode*);
Astnode* ast_params(Cstnode*, int*);
Astnode* ast_prog(Cstnode*);
void print_ast(Astnode*, int);
void free_ast(Astnode*);
void AstVisitorDispatch(ASTVisitor*, Astnode*, void*);
bool is_lval(Astnode*);

#endif  // CMMC_AST_H

