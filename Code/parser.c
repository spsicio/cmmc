#include <stdbool.h>
#include "parser.h"

Token last_token;

void read_token() {
  last_token = get_token();
}

int get_token_precedence() {
  switch (last_token) {
    case kASSIGNOP: return 1;
    case kRELOP: return 4;
    case kPLUS: return 5;
    case kMINUS: return 5;
    case kSTAR: return 6;
    case kDIV: return 6;
    case kAND: return 3;
    case kOR: return 2;
    default: return -1;
  }
}

Astnode* new_lex_node() {
  Astnode *p = alloc_lex_node(token_name[last_token], last_token, lineno);
  read_token();
  return p;
}

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

Astnode* parser_paren_exp() {
  int cur_lineno = lineno;
  Astnode *p_lp = new_lex_node();
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL) {
    // TODO
    free1(p_lp);
    return NULL;
  }
  if (last_token != kRP) {
    // TODO
    free2(p_lp, p_exp);
    return NULL;
  }
  Astnode *p_rp = new_lex_node();
  Astnode *p = new_syntax_node("Exp", 3);
  build3(p, p_rp, p_exp, p_lp); 
  return p;
}

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
      // TODO
      free2(p_id, p_lp);
      return NULL;
    }
    if (last_token != kRP) {
      // TODO
      free3(p_id, p_lp, p_args);
      return NULL;
    }
    Astnode *p_rp = new_lex_node();
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
    // TODO
    free1(p_uop);
    return NULL;
  }
  Astnode *p = new_syntax_node("Exp", 2);
  build2(p, p_exp, p_uop);
  return p;
}

Astnode* parser_op_rhs(int lst_prec, Astnode *p_lhs) {
  int cur_lineno = lineno;
  for (;;) {
    int cur_prec = get_token_precedence();
    if (cur_prec <= lst_prec) return p_lhs;
    Astnode *p_bop = new_lex_node();
    Astnode *p_rhs = parser_primary();
    if (p_rhs == NULL) {
      // TODO
      return NULL;
    }
    int nxt_prec = get_token_precedence();
    if (cur_prec < nxt_prec) {
      p_rhs = parser_op_rhs(cur_prec, p_rhs);
      if (p_rhs == NULL) {
        // TODO
        return NULL;
      }
    }
    Astnode *p_tmp = new_syntax_node("Exp", 3);
    build3(p_tmp, p_rhs, p_bop, p_lhs);
    p_lhs = p_tmp;
  }
  return p_lhs;
}

Astnode* parser_primary() {
  int cur_lineno = lineno;
  Astnode *p;
  switch (last_token) {
    case kINT: case kFLOAT: p = parser_val_exp(); break;
    case kID: p = parser_id_exp(); break;
    case kLP: p = parser_paren_exp(); break;
    case kMINUS: case kNOT: p = parser_uop_exp(); break;
    default: return NULL;
  }
  while (last_token == kDOT || last_token == kLB) {
    Astnode *p_infix = new_lex_node();
    if (p_infix->type == kDOT) {
      if (last_token != kID) {
        // TODO
        free2(p, p_infix);
        return NULL;
      }
      Astnode *p_id = new_lex_node();
      Astnode *p_tmp = new_syntax_node("Exp", 3);
      build3(p_tmp, p_id, p_infix, p);
      p = p_tmp;
    } else {
      Astnode *p_exp = parser_exp();
      if (p_exp == NULL) {
        // TODO
        free2(p, p_infix);
        return NULL;
      }
      if (last_token != kRB) {
        // TODO
        free3(p, p_infix, p_exp);
        return NULL;
      }
      Astnode *p_rb = new_lex_node();
      Astnode *p_tmp = new_syntax_node("Exp", 4);
      build4(p_tmp, p_rb, p_exp, p_infix, p);
      p = p_tmp;
    }
  }
  return p;
}

Astnode* parser_exp() {
  Astnode *p_lhs = parser_primary();
  if (p_lhs == NULL) {
    //TODO
    return NULL;
  }
  return parser_op_rhs(0, p_lhs);
}

Astnode* parser_vardec() {
  int cur_lineno = lineno;
  if (last_token != kID) {
    // TODO
    return NULL;
  }
  Astnode *p_id = new_lex_node();
  Astnode *p = new_syntax_node("VarDec", 1);
  build1(p, p_id);
  while (last_token == kLB) {
    Astnode *p_lb = new_lex_node();
    if (last_token != kINT) {
      // TODO
      free2(p, p_lb);
      return NULL;
    }
    Astnode *p_int = new_lex_node();
    if (last_token != kRB) {
      // TODO
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

Astnode* parser_dec() {
  int cur_lineno = lineno;
  Astnode *p_vardec = parser_vardec();
  if (p_vardec == NULL) {
    // TODO
    return NULL;
  }
  if (last_token == kASSIGNOP) {
    Astnode *p_assign = new_lex_node();
    Astnode *p_exp = parser_exp();
    if (p_exp == NULL) {
      // TODO
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

Astnode* parser_def() {
  int cur_lineno = lineno;
  Astnode *p_spec = parser_specifier();
  if (p_spec == NULL) {
    // TODO
    return NULL;
  }
  Astnode *p_declist = parser_declist();
  if (p_declist == NULL) {
    // TODO
    free1(p_spec);
    return NULL;
  }
  if (last_token != kSEMI) {
    // TODO
    free2(p_spec, p_declist);
    return NULL;
  }
  Astnode *p_semi = new_lex_node();
  Astnode *p = new_syntax_node("Def", 3);
  build3(p, p_semi, p_declist, p_spec);
  return p;
}

Astnode* parser_extdef() {
  if (!parser_specifier())
    return 1;
  if (last_token != kID)
    return 1;
  read_token();
  if (last_token == kSEMI)
    return 0;
  if (last_token == kLP) {
    read_token();
    if (last_token == kRP) {
      read_token();
      if (!parser_compst())
        return 1;
      return 0;
    } else {
      if (!parser_varlist())
        return 1;
      if (last_token != kRP)
        return 1;
      if (!parser_compst())
        return 1;
      return 0;
    }
  } else {
    if (last_token == kLB) {
    }
    if (last_token == kCOMMA) {
      if (!parser_extdeclist())
        return 1;
    }
    if (last_token != kSEMI)
      return 1;
    return 0;
  }
}

Astnode* parser_fundec() {
  if (last_token != kID) {
    // TODO
    return NULL;
  }
  int cur_lineno = lineno;
  Astnode *p_id = new_lex_node();
  if (last_token != kLP) {
    // TODO
    free1(p_id);
    return NULL;
  }
  Astnode *p_lp = new_lex_node();
  if (last_token == kRP) {
    Astnode *p_rp = new_lex_node();
    Astnode *p = new_syntax_node("FunDec", 3);
    build3(p, p_rp, p_lp, p_id);
    return p;
  } else {
    Astnode *p_varlist = parser_varlist();
    if (p_varlist == NULL) {
      // TODO
      free2(p_id, p_lp);
      return NULL;
    }
    if (last_token != kRP) {
      // TODO
      free3(p_id, p_lp, p_varlist);
      return NULL;
    }
    Astnode *p_rp = new_lex_node();
    Astnode *p = new_syntax_node("FunDec", 4);
    build4(p, p_rp, p_varlist, p_lp, p_id);
    return p;
  }
}

Astnode* parser_paramdec() {
  int cur_lineno = lineno;
  Astnode *p_spec = parser_specifier();
  if (p_spec == NULL) {
    // TODO
    return NULL;
  }
  Astnode *p_vardec = parser_vardec();
  if (p_vardec == NULL) {
    // TODO
    return NULL;
  }
  Astnode *p = new_syntax_node("ParamDec", 2);
  build2(p, p_vardec, p_spec);
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
    if (last_token != kLC) {
      if (p_id != NULL) {
        Astnode *p = new_syntax_node("Specifier", 1);
        Astnode *p_struct_spec = new_syntax_node("StructSpecifier", 2);
        Astnode *p_tag = new_syntax_node("Tag", 1);
        build1(p, p_struct_spec);
        build2(p_struct_spec, p_tag, p_struct);
        build1(p_tag, p_id);
        return p;
      } else {
        // TODO
        free1(p_struct);
        return NULL;
      }
    }
    Astnode *p_lc = new_lex_node();
    Astnode *p_deflist = parser_deflist();
    if (p_deflist == NULL) {
      // TODO
      free3(p_struct, p_id, p_lc);
      return NULL;
    }
    if (last_token != kRC) {
      // TODO
      free4(p_struct, p_id, p_lc, p_deflist);
      return NULL;
    }
    Astnode *p_rc = new_lex_node();
    if (p_id == NULL) {
      Astnode *p = new_syntax_node("Specifier", 1);
      Astnode *p_struct_spec = new_syntax_node("StructSpecifier", 4);
      build1(p, p_struct_spec);
      build4(p_struct_spec, p_rc, p_deflist, p_lc, p_struct);
      return p;
    } else {
      Astnode *p = new_syntax_node("Specifier", 1);
      Astnode *p_struct_spec = new_syntax_node("StructSpecifier", 5);
      Astnode *p_opttag = new_syntax_node("OptTag", 1);
      build1(p, p_struct_spec);
      build5(p_struct_spec, p_rc, p_deflist, p_lc, p_opttag, p_struct);
      build1(p_opttag, p_id);
      return p;
    }
  } else {
    // TODO
    return NULL;
  }
}

Astnode* parser_compst() {
  if (last_token != kLC) {
    // TODO
    return NULL;
  }
  int cur_lineno = lineno;
  Astnode *p_lc = new_lex_node();
  Astnode *p_deflist = parser_deflist();
  Astnode *p_stmtlist = parser_stmtlist();
  if (last_token != kRC) {
    // TODO
    free3(p_lc, p_deflist, p_stmtlist);
    return NULL;
  }
  Astnode *p_rc = new_lex_node();
  if (p_deflist == NULL && p_stmtlist == NULL) {
    Astnode *p = new_syntax_node("CompSt", 2);
    build2(p, p_rc, p_lc);
    return p;
  } else if (p_deflist == NULL) {
    Astnode *p = new_syntax_node("CompSt", 3);
    build3(p, p_rc, p_deflist, p_lc);
    return p;
  } else if (p_stmtlist == NULL) {
    Astnode *p = new_syntax_node("CompSt", 3);
    build3(p, p_rc, p_stmtlist, p_lc);
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
    // TODO
    free1(p_return);
    return NULL;
  }
  if (last_token != kSEMI) {
    // TODO
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
  Astnode *p_if = new_lex_node();
  if (last_token != kLP) {
    // TODO
    free1(p_if);
    return NULL;
  }
  Astnode *p_lp = new_lex_node();
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL) {
    // TODO
    free2(p_if, p_lp);
    return NULL;
  }
  if (last_token != kRP) {
    // TODO
    free3(p_if, p_lp, p_exp);
    return NULL;
  }
  Astnode *p_rp = new_lex_node();
  Astnode *p_stmt = parser_stmt();
  if (p_stmt == NULL) {
    // TODO
    free4(p_if, p_lp, p_exp, p_rp);
    return NULL;
  }
  if (last_token == kELSE) {
    Astnode *p_else = new_lex_node();
    Astnode *p_else_stmt = parser_stmt();
    if (p_else_stmt == NULL) {
      // TODO
      free5(p_if, p_lp, p_exp, p_rp, p_else);
      return NULL;
    }
    Astnode *p = new_syntax_node("Stmt", 7);
    build7(p, p_else_stmt, p_else, p_stmt, p_rp, p_exp, p_lp, p_if);
    return p;
  } else {
    Astnode *p = new_syntax_node("Stmt", 5);
    build5(p, p_stmt, p_rp, p_exp, p_lp, p_if);
    return p;
  }
}

Astnode* parser_while_stmt() {
  int cur_lineno = lineno;
  Astnode *p_while = new_lex_node();
  if (last_token != kLP) {
    // TODO
    free1(p_while);
    return NULL;
  }
  Astnode *p_lp = new_lex_node();
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL) {
    // TODO
    free2(p_while, p_lp);
    return NULL;
  }
  if (last_token != kRP) {
    // TODO
    free3(p_while, p_lp, p_exp);
    return NULL;
  }
  Astnode *p_rp = new_lex_node();
  Astnode *p_stmt = parser_stmt();
  if (p_stmt == NULL) {
    // TODO
    free4(p_while, p_lp, p_exp, p_rp);
    return NULL;
  }
  Astnode *p = new_syntax_node("Exp", 5);
  build5(p, p_stmt, p_rp, p_exp, p_lp, p_while);
  return p;
}

Astnode* parser_stmt() {
  int cur_lineno = lineno;
  switch (last_token) {
    case kLC: {
      Astnode *p_compst = parser_compst();
      if (p_compst == NULL) {
        // TODO
        return NULL;
      }
      Astnode *p = new_syntax_node("Stmt", 1);
      build1(p, p_compst);
      return p;
    }
    case kRETURN: return parser_return_stmt();
    case kIF: return parser_if_stmt();
    case kWHILE: return parser_while_stmt();
    default: {
      Astnode *p_exp = parser_exp();
      if (p_exp == NULL) {
        // TODO
        return NULL;
      }
      if (last_token != kSEMI) {
        // TODO
        free1(p_exp);
        return NULL;
      }
      Astnode *p_semi = new_lex_node();
      Astnode *p = new_syntax_node("Stmt", 2);
      build2(p, p_semi, p_exp);
      return p;
    }
  }
}

Astnode* parser_args() {
  int cur_lineno = lineno;
  Astnode *p_exp = parser_exp();
  if (p_exp == NULL) {
    // TODO
    return NULL;
  }
  if (last_token == kCOMMA) {
    Astnode *p_comma = new_lex_node();
    Astnode *p_args = parser_args();
    if (p_args == NULL) {
      // TODO
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

Astnode* parser_varlist() {
  int cur_lineno = lineno;
  Astnode *p_paramdec = parser_paramdec();
  if (p_paramdec == NULL) {
    // TODO
    return NULL;
  }
  if (last_token == kCOMMA) {
    Astnode *p_comma = new_lex_node();
    Astnode *p_varlist = parser_varlist();
    if (p_varlist == NULL) {
      // TODO
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

Astnode* parser_declist() {
  int cur_lineno = lineno;
  Astnode *p_dec = parser_dec();
  if (parser_dec == NULL) {
    // TODO
    return NULL;
  }
  if (last_token == kCOMMA) {
    Astnode *p_comma = new_lex_node();
    Astnode *p_declist = parser_declist();
    if (p_declist == NULL) {
      // TODO
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

Astnode* parser_deflist() {
  if (last_token != kTYPE && last_token != kSTRUCT) return NULL;
  int cur_lineno = lineno;
  Astnode *p_def = parser_def();
  if (p_def == NULL) {
    // TODO
    return NULL;
  }
  Astnode *p_deflist = parser_deflist();
  if (p_deflist == NULL) {
    Astnode *p = new_syntax_node("Deflist", 1);
    build1(p, p_def);
    return p;
  } else {
    Astnode *p = new_syntax_node("Deflist", 2);
    build2(p, p_deflist, p_def);
    return p;
  }
}

Astnode* parser_extdeclist() {
  int cur_lineno = lineno;
  Astnode *p_vardec = parser_vardec();
  if (p_vardec == NULL) {
    // TODO
    return NULL;
  }
  if (last_token == kCOMMA) {
    Astnode *p_comma = new_lex_node();
    Astnode *p_extdeclist = parser_extdeclist();
    if (p_extdeclist == NULL) {
      // TODO
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

Astnode* parser_extdeflist() {
  if (last_token == kTYPE || last_token == kSTRUCT) {
    if (!parser_extdef())
      return 1;
    if (!parser_extdeflist())
      return 1;
  }
  return 0;
}

Astnode* parser_stmtlist() {
  if (last_token == kRC || last_token == kEOF) return NULL;
  int cur_lineno = lineno;
  Astnode *p_stmt = parser_stmt();
  if (p_stmt == NULL) {
    // TODO
    return NULL;
  }
  Astnode *p_stmtlist = parser_stmtlist();
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

Astnode* parser_program() {
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

