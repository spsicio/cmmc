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
  if (!parser_specifier())
    return 1;
  if (!parser_declist())
    return 1;
  return 0;
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

Astnode* parser_paramdec() {
  if (!parser_specifier())
    return 1;
  if (!parser_vardec())
    return 1;
  return 0;
}

Astnode* parser_specifier() {
  if (last_token == kTYPE) {
    return 0; 
  } else if (last_token == kSTRUCT) {
    bool ever_ID = false;
    read_token();
    if (last_token == kID) {
      read_token();
      ever_ID = 1;
    }
    if (last_token != kLC) {
      if (ever_ID) {
        return 0;
      }
      return 1;
    }
    read_token();
    if (parser_deflist()) {
      return 1;
    }
    if (last_token != kRC) {
      return 1;
    }
    read_token();
    return 0;
  } else {
    return 1;
  }
}

Astnode* parser_compst() {
  if (last_token != kLC)
    return 1;
  read_token();
  if (!parser_deflist())
    return 1;
  if (!parser_stmtlist())
    return 1;
  if (last_token != kRC)
    return 1;
  read_token();
  return 0;
}

Astnode* parser_stmt() {
  switch (last_token) {
    case kLC: {
      if (!parser_compst())
        return 1;
      return 0;
    }
    case kRETURN: {
      read_token();
      if (!parser_exp())
        return 1;
      if (last_token != kSEMI)
        return 1;
      read_token();
      return 0;
    }
    case kIF: {
      read_token();
      if (last_token != kLP)
        return 1;
      read_token();
      if (!parser_exp())
        return 1;
      if (last_token != kRP)
        return 1;
      read_token();
      if (!parser_stmt())
        return 1;
      if (last_token == kELSE) {
        if (!parser_stmt())
          return 1;
      }
      return 0;
    }
    case kWHILE: {
      read_token();
      if (last_token != kLP)
        return 1;
      read_token();
      if (!parser_exp())
        return 1;
      if (last_token != kRP)
        return 1;
      read_token();
      if (!parser_stmt())
        return 1;
      return 0;
    }
    default: {
      if (!parser_exp())
        return 1;
      if (last_token != kSEMI)
        return 1;
      return 0;
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
  if (!parser_paramdec())
    return 1;
  if (last_token == kCOMMA) {
    read_token();
    if (!parser_varlist())
      return 1;
  }
  return 0;
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
  if (last_token == kTYPE || last_token == kSTRUCT) {
    if (!parser_def())
      return 1;
    if (!parser_deflist())
      return 1;
  }
  return 0; 
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
  if (last_token != kRC) {
    if (!parser_stmt())
      return 1;
    if (!parser_stmtlist())
      return 1;
  }
  return 0;
}

Astnode* parser_program() {
  read_token();
  if (!parser_extdeflist())
    return 1;
  return 0;
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

