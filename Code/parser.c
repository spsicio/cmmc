#include <stdbool.h>
#include <stdio.h>
#include "parser.h"

Token last_token;

void read_token() {
  last_token = get_token();
}

int get_token_precedence() {
  switch (last_token) {
    case kEOF: return 0;
    case kSEMI: case kCOMMA: return 0;
    case kASSIGNOP: return 2;
    case kRELOP: return 5;
    case kPLUS: return 6;
    case kMINUS: return 6;
    case kSTAR: return 7;
    case kDIV: return 7;
    case kAND: return 4;
    case kOR: return 3;
    case kTYPE: return 0;
    case kRP: case kRB: case kRC: return 0;
    case kSTRUCT: return 0;
    case kRETURN: case kIF: case kELSE: case kWHILE: return 0;
    default: return 1;
  }
}

Astnode* new_lex_node() {
  Astnode *p = alloc_lex_node(token_name[last_token], last_token, lineno);
  read_token();
  return p;
}

Astnode* parser_token(Token tgt) {
  return last_token == tgt ? new_lex_node() : NULL;
}

#define log_error(s) do { \
      ++error_cnt; \
      printf("Error type B at Line %d: %s (in %s)\n", lineno, s, __func__); \
    } while (0)

#define new_syntax_node(name, chd_num) alloc_syntax_node(name, cur_lineno, chd_num)
#define build1(p, s0)                              (p)->chd[0] = (s0)
#define build2(p, s1, ...) build1(p, __VA_ARGS__); (p)->chd[1] = (s1)
#define build3(p, s2, ...) build2(p, __VA_ARGS__); (p)->chd[2] = (s2)
#define build4(p, s3, ...) build3(p, __VA_ARGS__); (p)->chd[3] = (s3)
#define build5(p, s4, ...) build4(p, __VA_ARGS__); (p)->chd[4] = (s4)
#define build6(p, s5, ...) build5(p, __VA_ARGS__); (p)->chd[5] = (s5)
#define build7(p, s6, ...) build6(p, __VA_ARGS__); (p)->chd[6] = (s6)
#define free1(s0)                          free_ast(s0)
#define free2(s1, ...) free1(__VA_ARGS__); free_ast(s1)
#define free3(s2, ...) free2(__VA_ARGS__); free_ast(s2)
#define free4(s3, ...) free3(__VA_ARGS__); free_ast(s3)
#define free5(s4, ...) free4(__VA_ARGS__); free_ast(s4)
#define free6(s5, ...) free5(__VA_ARGS__); free_ast(s5)
#define free7(s6, ...) free6(__VA_ARGS__); free_ast(s6)

// ==========
// EXPRESSION
// ==========

Astnode* parser_val_exp() {
  int cur_lineno = lineno;
  Astnode *p_val = new_lex_node(last_token);
  Astnode *p = new_syntax_node("Exp", 1);
  build1(p, p_val);
  return p;
}

Astnode* parser_id_exp() {
  int cur_lineno = lineno;
  Astnode *p_id = new_lex_node();
  if (last_token == kLP) {
    Astnode *p_lp = new_lex_node();
    if (last_token == kRP) {
      Astnode *p_rp = new_lex_node();
      Astnode *p = new_syntax_node("Exp", 3);
      build3(p, p_rp, p_lp, p_id);
      return p;
    }
    Astnode *p_args = parser_args();
    if (p_args == NULL) {
      free2(p_id, p_lp);
      return NULL;
    }
    Astnode *p_rp = parser_token(kRP);
    if (p_rp == NULL) {
      free3(p_id, p_lp, p_args);
      return NULL;
    }
    Astnode *p = new_syntax_node("Exp", 4);
    build4(p, p_rp, p_args, p_lp, p_id);
    return p;
  }
  Astnode *p = new_syntax_node("Exp", 1);
  build1(p, p_id);
  return p;
}

Astnode* parser_uop_exp() {
  int cur_lineno = lineno;
  Astnode *p_uop = new_lex_node();
  Astnode *p_exp = parser_primary();
  if (p_exp == NULL) {
    free1(p_uop);
    return NULL;
  }
  Astnode *p = new_syntax_node("Exp", 2);
  build2(p, p_exp, p_uop);
  return p;
}

Astnode* parser_paren_exp() {
  int cur_lineno = lineno;
  Astnode *p_lp = new_lex_node();
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL) {
    free1(p_lp);
    return NULL;
  }
  Astnode *p_rp = parser_token(kRP);
  if (p_rp == NULL) {
    free2(p_lp, p_exp);
    return NULL;
  }
  Astnode *p = new_syntax_node("Exp", 3);
  build3(p, p_rp, p_exp, p_lp); 
  return p;
}

Astnode* parser_primary() {
  int cur_lineno = lineno;
  Astnode *p = NULL;
  switch (last_token) {
    case kINT: case kFLOAT: p = parser_val_exp(); break;
    case kID: p = parser_id_exp(); break;
    case kLP: p = parser_paren_exp(); break;
    case kMINUS: case kNOT: p = parser_uop_exp(); break;
    default: return NULL;
  }
  if (p == NULL) return NULL;
  while (last_token == kDOT || last_token == kLB) {
    Astnode *p_infix = new_lex_node();
    if (p_infix->type == kDOT) {
      Astnode *p_id = parser_token(kID);
      if (p_id == NULL) {
        free2(p, p_infix);
        return NULL;
      }
      Astnode *p_tmp = new_syntax_node("Exp", 3);
      build3(p_tmp, p_id, p_infix, p);
      p = p_tmp;
    } else {
      Astnode *p_exp = parser_exp();
      if (p_exp == NULL) {
        free2(p, p_infix);
        return NULL;
      }
      Astnode *p_rb = parser_token(kRB);
      if (p_rb == NULL) {
        free3(p, p_infix, p_exp);
        return NULL;
      }
      Astnode *p_tmp = new_syntax_node("Exp", 4);
      build4(p_tmp, p_rb, p_exp, p_infix, p);
      p = p_tmp;
    }
  }
  return p;
}

Astnode* parser_op_rhs(int lst_prec, Astnode *p_lhs) {
  int cur_lineno = p_lhs->lineno;
  for (;;) {
    int cur_prec = get_token_precedence();
    if (cur_prec < lst_prec || (cur_prec == lst_prec && cur_prec != 2)) {
      return p_lhs;
    }
    if (cur_prec == 1) {
      free1(p_lhs);
      return NULL;
    }
    Astnode *p_bop = new_lex_node();
    Astnode *p_rhs = parser_primary();
    if (p_rhs == NULL) {
      free1(p_lhs);
      return NULL;
    }
    int nxt_prec = get_token_precedence();
    if (cur_prec < nxt_prec || (cur_prec == nxt_prec && cur_prec == 2)) {
      p_rhs = parser_op_rhs(cur_prec, p_rhs);
      if (p_rhs == NULL) {
        free1(p_lhs);
        return NULL;
      }
    }
    Astnode *p_tmp = new_syntax_node("Exp", 3);
    build3(p_tmp, p_rhs, p_bop, p_lhs);
    p_lhs = p_tmp;
  }
  return p_lhs;
}

Astnode* parser_exp() {
  Astnode *p_lhs = parser_primary();
  if (p_lhs == NULL) return NULL;
  return parser_op_rhs(0, p_lhs);
}

// =========
// STATEMENT
// =========

Astnode* parser_compst() {
  int cur_lineno = lineno;
  Astnode *p_lc = parser_token(kLC);
  if (p_lc == NULL) {
    log_error("Missing '{'.");
    while (last_token != kRC && last_token != kEOF) read_token();
    read_token();
    return NULL;
  }
  Astnode *p_deflist = NULL, *p_stmtlist = NULL;
  for (;;) {
    if (last_token == kRC) break;
    if (last_token == kTYPE || last_token == kSTRUCT) {
      Astnode *p_def = parser_def();
      if (p_def != NULL) {
        p_deflist = parser_deflist(p_def, p_def->lineno, false);
        p_stmtlist = parser_stmtlist(NULL, lineno);
        break;
      }
    } else {
      Astnode *p_stmt = parser_stmt();
      if (p_stmt != NULL) {
        p_stmtlist = parser_stmtlist(p_stmt, p_stmt->lineno);
        break;
      }
    }
  }
  Astnode *p_rc = parser_token(kRC);
  if (p_rc == NULL) {
    log_error("Missing '}'.");
    free3(p_lc, p_deflist, p_stmtlist);
    return NULL;
  }
  if (p_deflist == NULL && p_stmtlist == NULL) {
    Astnode *p = new_syntax_node("CompSt", 2);
    build2(p, p_rc, p_lc);
    return p;
  } else if (p_deflist == NULL) {
    Astnode *p = new_syntax_node("CompSt", 3);
    build3(p, p_rc, p_stmtlist, p_lc);
    return p;
  } else if (p_stmtlist == NULL) {
    Astnode *p = new_syntax_node("CompSt", 3);
    build3(p, p_rc, p_deflist, p_lc);
    return p;
  } else {
    Astnode *p = new_syntax_node("CompSt", 4);
    build4(p, p_rc, p_stmtlist, p_deflist, p_lc);
    return p;
  }
}

Astnode* parser_return_stmt() {
  int cur_lineno = lineno;
  Astnode *p_return = new_lex_node();
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL) {
    log_error("Invalid Expression.");
    while (last_token != kSEMI && last_token != kEOF) read_token();
    read_token();
    return NULL;
  }
  if (last_token != kSEMI) {
    log_error("Missing ';'.");
    free2(p_return ,p_exp);
    return NULL;
  }
  Astnode *p_semi = new_lex_node(); 
  Astnode *p = new_syntax_node("Stmt", 3);
  build3(p, p_semi, p_exp, p_return);
  return p; 
}

Astnode* parser_if_stmt() {
  int cur_lineno = lineno;
  bool ever_error = false;
  Astnode *p_if = new_lex_node();
  Astnode *p_lp = parser_token(kLP);
  if (p_lp == NULL) {
    log_error("Missing '('.");
    ever_error = true;
  }
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL) {
    log_error("Invalid Expression.");
    ever_error = true;
    while (last_token != kRP && last_token != kEOF) read_token();
  }
  Astnode *p_rp = parser_token(kRP);
  if (p_rp == NULL) {
    log_error("Missing ')'.");
    ever_error = true;
  }
  Astnode *p_stmt = parser_stmt();
  if (p_stmt == NULL) ever_error = true;
  if (last_token == kELSE) {
    Astnode *p_else = new_lex_node();
    Astnode *p_else_stmt = parser_stmt();
    if (p_else_stmt == NULL) ever_error = true;
    if (ever_error) {
      free7(p_if, p_lp, p_exp, p_rp, p_stmt, p_else, p_else_stmt);
      return NULL;
    }
    Astnode *p = new_syntax_node("Stmt", 7);
    build7(p, p_else_stmt, p_else, p_stmt, p_rp, p_exp, p_lp, p_if);
    return p;
  } else {
    if (ever_error) {
      free5(p_if, p_lp, p_exp, p_rp, p_stmt);
      return NULL;
    }
    Astnode *p = new_syntax_node("Stmt", 5);
    build5(p, p_stmt, p_rp, p_exp, p_lp, p_if);
    return p;
  }
}

Astnode* parser_while_stmt() {
  int cur_lineno = lineno;
  bool ever_error = false;
  Astnode *p_while = new_lex_node();
  Astnode *p_lp = parser_token(kLP);
  if (p_lp == NULL) {
    log_error("Missing '('.");
    ever_error = true;
  }
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL) {
    log_error("Invalid Expression.");
    ever_error = true;
    while (last_token != kRP && last_token != kEOF)
      read_token();
  }
  Astnode *p_rp = parser_token(kRP);
  if (p_rp == NULL) {
    log_error("Missing ')'.");
    ever_error = true;
  }
  Astnode *p_stmt = parser_stmt();
  if (p_stmt == NULL) ever_error = true;
  if (ever_error) {
    free5(p_while, p_lp, p_exp, p_rp, p_stmt);
    return NULL;
  }
  Astnode *p = new_syntax_node("Stmt", 5);
  build5(p, p_stmt, p_rp, p_exp, p_lp, p_while);
  return p;
}

Astnode* parser_exp_stmt() {
  int cur_lineno = lineno; 
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL || last_token == kRP || last_token == kRB) {
    log_error("Invalid Expression.");
    while (last_token != kSEMI && last_token != kEOF) read_token();
    read_token();
    free1(p_exp);
    return NULL;
  }
  Astnode *p_semi = parser_token(kSEMI);
  if (p_semi == NULL) {
    log_error("Missing ';'.");
    free1(p_exp);
    return NULL;
  }
  Astnode *p = new_syntax_node("Stmt", 2);
  build2(p, p_semi, p_exp);
  return p;
}

Astnode* parser_stmt() {
  switch (last_token) {
    case kLC: {
      int cur_lineno = lineno;
      Astnode *p_compst = parser_compst();
      if (p_compst == NULL) return NULL;
      Astnode *p = new_syntax_node("Stmt", 1);
      build1(p, p_compst);
      return p;
    }
    case kRETURN: return parser_return_stmt();
    case kIF: return parser_if_stmt();
    case kWHILE: return parser_while_stmt();
    default: return parser_exp_stmt();
  }
}

Astnode* parser_stmtlist(Astnode *p_stmt, int cur_lineno) {
  if (p_stmt == NULL) {
    if (last_token == kRC || last_token == kEOF) return NULL;
    p_stmt = parser_stmt();
  }
  Astnode *p_stmtlist = parser_stmtlist(NULL, lineno);
  if (p_stmt == NULL) return NULL;
  if (p_stmtlist == NULL) {
    Astnode *p = new_syntax_node("StmtList", 1);
    build1(p, p_stmt);
    return p;
  } else {
    Astnode *p = new_syntax_node("StmtList", 2);
    build2(p, p_stmtlist, p_stmt);
    return p;
  }
}

// =========
// COMPONENT
// =========

Astnode* parser_args() {
  int cur_lineno = lineno;
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL) return NULL;
  if (last_token == kCOMMA) {
    Astnode *p_comma = new_lex_node();
    Astnode *p_args = parser_args();
    if (p_args == NULL) {
      free2(p_exp, p_comma);
      return NULL;
    }
    Astnode *p = new_syntax_node("Args", 3);
    build3(p, p_args, p_comma, p_exp);
    return p;
  }
  Astnode *p = new_syntax_node("Args", 1);
  build1(p, p_exp);
  return p;
}

Astnode* parser_paramdec() {
  int cur_lineno = lineno;
  Astnode *p_spec = parser_specifier();
  if (p_spec == NULL) return NULL;
  Astnode *p_vardec = parser_vardec(NULL, lineno);
  if (p_vardec == NULL) return NULL;
  Astnode *p = new_syntax_node("ParamDec", 2);
  build2(p, p_vardec, p_spec);
  return p;
}

Astnode* parser_varlist() {
  if (last_token == kRP) return NULL;
  int cur_lineno = lineno;
  Astnode *p_paramdec = parser_paramdec();
  if (p_paramdec == NULL) return NULL;
  if (last_token == kCOMMA) {
    Astnode *p_comma = new_lex_node();
    Astnode *p_varlist = parser_varlist();
    if (p_varlist == NULL) {
      free2(p_paramdec, p_comma);
      return NULL;
    }
    Astnode *p = new_syntax_node("VarList", 3);
    build3(p, p_varlist, p_comma, p_paramdec);
    return p;
  } else {
    Astnode *p = new_syntax_node("VarList", 1);
    build1(p, p_paramdec);
    return p;
  }
}

Astnode* parser_fundec(Astnode *p_id, int cur_lineno) {
  Astnode *p_lp = new_lex_node();
  if (last_token == kRP) {
    Astnode *p_rp = new_lex_node();
    Astnode *p = new_syntax_node("FunDec", 3);
    build3(p, p_rp, p_lp, p_id);
    return p;
  } else {
    Astnode *p_varlist = parser_varlist();
    if (p_varlist == NULL) {
      free2(p_id, p_lp);
      return NULL;
    }
    Astnode *p_rp = parser_token(kRP);
    if (p_rp == NULL) {
      free3(p_id, p_lp, p_varlist);
      return NULL;
    }
    Astnode *p = new_syntax_node("FunDec", 4);
    build4(p, p_rp, p_varlist, p_lp, p_id);
    return p;
  }
}

Astnode* parser_vardec(Astnode *p_id, int cur_lineno) {
  if (p_id == NULL) {
    p_id = parser_token(kID);
    if (p_id == NULL) return NULL;
  }
  Astnode *p = new_syntax_node("VarDec", 1);
  build1(p, p_id);
  while (last_token == kLB) {
    Astnode *p_lb = new_lex_node();
    if (last_token != kINT) {
      free2(p, p_lb);
      return NULL;
    }
    Astnode *p_int = new_lex_node();
    if (last_token != kRB) {
      free3(p, p_lb, p_int);
      return NULL;
    }
    Astnode *p_rb = new_lex_node();
    Astnode *p_tmp = new_syntax_node("VarDec", 4);
    build4(p_tmp, p_rb, p_int, p_lb, p);
    p = p_tmp;
  }
  return p;
}

Astnode* parser_extdeclist(Astnode *p_vardec, int cur_lineno) {
  if (p_vardec == NULL) {
    p_vardec = parser_vardec(NULL, cur_lineno);
    if (p_vardec == NULL) return NULL;
  }
  if (last_token == kCOMMA) {
    Astnode *p_comma = new_lex_node();
    Astnode *p_extdeclist = parser_extdeclist(NULL, lineno);
    if (p_extdeclist == NULL) {
      free2(p_vardec, p_comma);
      return NULL;
    }
    Astnode *p = new_syntax_node("ExtDecList", 3);
    build3(p, p_extdeclist, p_comma, p_vardec);
    return p;
  }
  Astnode *p = new_syntax_node("ExtDecList", 1);
  build1(p, p_vardec);
  return p;
}

Astnode* parser_dec() {
  int cur_lineno = lineno;
  Astnode *p_vardec = parser_vardec(NULL, lineno);
  if (p_vardec == NULL) return NULL;
  if (last_token == kASSIGNOP) {
    Astnode *p_assign = new_lex_node();
    Astnode *p_exp = parser_exp();
    if (p_exp == NULL) {
      free2(p_assign, p_exp);
      return NULL;
    }
    Astnode *p = new_syntax_node("Dec", 3);
    build3(p, p_exp, p_assign, p_vardec);
    return p;
  }
  Astnode *p = new_syntax_node("Dec", 1);
  build1(p, p_vardec);
  return p;
}

Astnode* parser_declist() {
  int cur_lineno = lineno;
  Astnode *p_dec = parser_dec();
  if (p_dec == NULL) return NULL;
  if (last_token == kCOMMA) {
    Astnode *p_comma = new_lex_node();
    Astnode *p_declist = parser_declist();
    if (p_declist == NULL) {
      free2(p_dec, p_comma);
      return NULL;
    }
    Astnode *p = new_syntax_node("DecList", 3);
    build3(p, p_declist, p_comma, p_dec);
    return p;
  }
  Astnode *p = new_syntax_node("DecList", 1);
  build1(p, p_dec);
  return p;
}

Astnode* parser_specifier() {
  int cur_lineno = lineno;
  if (last_token == kTYPE) {
    Astnode *p_type = new_lex_node();
    Astnode *p = new_syntax_node("Specifier", 1);
    build1(p, p_type);
    return p;
  } else if (last_token == kSTRUCT) {
    Astnode *p_struct = new_lex_node();
    Astnode *p_id = NULL;
    if (last_token == kID) p_id = new_lex_node();
    Astnode *p_lc = parser_token(kLC);
    if (p_lc == NULL) {
      if (p_id != NULL) {
        Astnode *p = new_syntax_node("Specifier", 1);
        Astnode *p_struct_spec = new_syntax_node("StructSpecifier", 2);
        Astnode *p_tag = new_syntax_node("Tag", 1);
        build1(p, p_struct_spec);
        build2(p_struct_spec, p_tag, p_struct);
        build1(p_tag, p_id);
        return p;
      } else {
        log_error("Missing Struct Name.");
        free1(p_struct);
        return NULL;
      }
    }
    Astnode *p_deflist = parser_deflist(NULL, lineno, true);
    Astnode *p_rc = parser_token(kRC);
    if (p_rc == NULL) {
      log_error("Missing '}'");
      free4(p_struct, p_id, p_lc, p_deflist);
      return NULL;
    }
    Astnode *p = new_syntax_node("Specifier", 1);
    if (p_id == NULL && p_deflist == NULL) {
      Astnode *p_struct_spec = new_syntax_node("StructSpecifier", 3);
      build1(p, p_struct_spec);
      build3(p_struct_spec, p_rc, p_lc, p_struct);
    } else if (p_id == NULL) {
      Astnode *p_struct_spec = new_syntax_node("StructSpecifier", 4);
      build1(p, p_struct_spec);
      build4(p_struct_spec, p_rc, p_deflist, p_lc, p_struct);
      return p;
    } else if (p_deflist == NULL) {
      Astnode *p_struct_spec = new_syntax_node("StructSpecifier", 4);
      Astnode *p_opttag = new_syntax_node("OptTag", 1);
      build1(p, p_struct_spec);
      build4(p_struct_spec, p_rc, p_lc, p_opttag, p_struct);
      build1(p_opttag, p_id);
    } else {
      Astnode *p_struct_spec = new_syntax_node("StructSpecifier", 5);
      Astnode *p_opttag = new_syntax_node("OptTag", 1);
      build1(p, p_struct_spec);
      build5(p_struct_spec, p_rc, p_deflist, p_lc, p_opttag, p_struct);
      build1(p_opttag, p_id);
    }
    return p;
  } else {
    log_error("Invalid Specifier Start.");
    return NULL;
  }
}

// ==========
// DEFINITION
// ==========

Astnode* parser_def() {
  int cur_lineno = lineno;
  bool ever_error = false;
  Astnode *p_spec = parser_specifier();
  if (p_spec == NULL) {
    while (last_token != kSEMI && last_token != kEOF) read_token();
    read_token();
    return NULL;
  }
  Astnode *p_declist = parser_declist();
  if (p_declist == NULL) {
    log_error("Invalid DecList.");
    while (last_token != kSEMI && last_token != kEOF) read_token();
    read_token();
    free1(p_spec);
    return NULL;
  }
  Astnode *p_semi = parser_token(kSEMI);
  if (p_semi == NULL) {
    log_error("Missing ';'.");
    free2(p_spec, p_declist);
    return NULL;
  }
  Astnode *p = new_syntax_node("Def", 3);
  build3(p, p_semi, p_declist, p_spec);
  return p;
}

Astnode* parser_deflist(Astnode *p_def, int cur_lineno, bool in_spec) {
  if (p_def == NULL) {
    if (( in_spec &&  last_token == kRC) ||
        (!in_spec && (last_token != kTYPE && last_token != kSTRUCT)))
      return NULL;
    p_def = parser_def();
  }
  Astnode *p_deflist = parser_deflist(NULL, lineno, in_spec);
  if (p_def == NULL) return NULL;
  if (p_deflist == NULL) {
    Astnode *p = new_syntax_node("DefList", 1);
    build1(p, p_def);
    return p;
  } else {
    Astnode *p = new_syntax_node("DefList", 2);
    build2(p, p_deflist, p_def);
    return p;
  }
}

Astnode* parser_extdef() {
  int cur_lineno = lineno;
  Astnode *p_spec = parser_specifier();
  if (p_spec == NULL) {
    while (last_token != kSEMI && last_token != kRC &&
           last_token != kEOF)
      read_token();
    read_token();
    return NULL;
  }
  if (last_token == kSEMI) {
    Astnode *p_semi = new_lex_node();
    Astnode *p = new_syntax_node("ExtDef", 2);
    build2(p, p_semi, p_spec);
    return p;
  }
  Astnode *p_id = parser_token(kID);
  if (p_id == NULL) {
    log_error("Expected a ID.");
    while (last_token != kSEMI && last_token != kRC &&
           last_token != kEOF)
      read_token();
    read_token();
    free1(p_spec);
    return NULL;
  }
  if (last_token == kLP) {
    Astnode *p_fundec = parser_fundec(p_id, lineno);
    if (p_fundec == NULL) {
      log_error("Invalid FunDec.");
      while (last_token != kRC && last_token != kEOF) read_token();
      read_token();
      free1(p_spec);
      return NULL;
    }
    Astnode *p_compst = parser_compst();
    if (p_compst == NULL) {
      free2(p_spec, p_fundec);
      return NULL;
    }
    Astnode *p = new_syntax_node("ExtDef", 3);
    build3(p, p_compst, p_fundec, p_spec);
    return p;
  } else {
    int nxt_lineno = lineno;
    Astnode *p_vardec = parser_vardec(p_id, nxt_lineno);
    Astnode *p_extdeclist = parser_extdeclist(p_vardec, nxt_lineno);
    if (p_extdeclist == NULL) {
      log_error("Invalid ExtDecList.");
      while (last_token != kSEMI && last_token != kEOF) read_token();
      read_token();
      free1(p_spec);
      return NULL;
    }
    Astnode *p_semi = parser_token(kSEMI);
    if (p_semi == NULL) {
      log_error("Missing ';'.");
      free2(p_spec, p_extdeclist);
      return NULL;
    }
    Astnode *p = new_syntax_node("ExtDef", 3);
    build3(p, p_semi, p_extdeclist, p_spec);
    return p;
  }
}

Astnode* parser_extdeflist() {
  if (last_token == kEOF) return NULL;
  int cur_lineno = lineno;
  Astnode *p_extdef = parser_extdef();
  if (p_extdef == NULL) {
    while (last_token != kTYPE && last_token != kSTRUCT &&
           last_token != kEOF) {
      read_token();
    }
  }
  Astnode *p_extdeflist = parser_extdeflist();
  if (p_extdef == NULL) return NULL;
  if (p_extdeflist == NULL) {
    Astnode *p = new_syntax_node("ExtDefList", 1);
    build1(p, p_extdef);
    return p;
  } else {
    Astnode *p = new_syntax_node("ExtDefList", 2);
    build2(p, p_extdeflist, p_extdef);
    return p;
  } 
}

Astnode* parser_program() {
  int cur_lineno = lineno;
  Astnode *p_extdeflist = parser_extdeflist();
  if (p_extdeflist == NULL) return NULL;
  Astnode *p = new_syntax_node("Program", 1);
  build1(p, p_extdeflist);
  return p;
}

#undef new_syntax_node
#undef build1
#undef build2
#undef build3
#undef build4
#undef build5
#undef build6
#undef build7
#undef free1
#undef free2
#undef free3
#undef free4
#undef free5
#undef free6
#undef free7

