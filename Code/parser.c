#include <stdbool.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

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
    case kDOT: return 7;
    case kLB: return 7;
    default: return -1;
  }
}

#define new_lex_node(token) alloc_lex_node(token_name[token], token, lineno)
#define new_syntax_node(name, chd_num) alloc_syntax_node(name, cur_lineno, chd_num)
#define build1(p, s0)                              (p)->chd[0] = (s0)
#define build2(p, s1, ...) build1(p, __VA_ARGS__); (p)->chd[1] = (s1)
#define build3(p, s2, ...) build2(p, __VA_ARGS__); (p)->chd[2] = (s2)
#define build4(p, s3, ...) build3(p, __VA_ARGS__); (p)->chd[3] = (s3)
#define build5(p, s4, ...) build4(p, __VA_ARGS__); (p)->chd[4] = (s4)
#define build6(p, s5, ...) build5(p, __VA_ARGS__); (p)->chd[5] = (s5)
#define build7(p, s6, ...) build6(p, __VA_ARGS__); (p)->chd[6] = (s6)
#define free1(p, s0)                             ast_free(s0)
#define free2(p, s1, ...) free1(p, __VA_ARGS__); ast_free(s1)
#define free3(p, s2, ...) free2(p, __VA_ARGS__); ast_free(s2)
#define free4(p, s3, ...) free3(p, __VA_ARGS__); ast_free(s3)
#define free5(p, s4, ...) free4(p, __VA_ARGS__); ast_free(s4)
#define free6(p, s5, ...) free5(p, __VA_ARGS__); ast_free(s5)

Astnode* parser_paren_exp() {
  int cur_lineno = lineno;
  Astnode *p_lp = new_lex_node(kLP);
  read_token();
  Astnode *p_exp = pasrser_exp();
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
  Astnode *p_rp = new_lex_node(kRP);
  read_token();
  Astnode *p = new_syntax_node("Exp", 3);
  build3(p, p_lp, p_exp, p_rp); 
  return p;
}

Astnode* parser_val_exp() {
  int cur_lineno = lineno;
  Astnode *p_val = new_lex_ndoe(last_token);
  Astonde *p = new_syntax_node("Exp", 1);
  build1(p, p_val);
  read_token();
  return p;
}

Astnode* parser_id_exp() {
  int cur_lineno = lineno;
  Astnode *p_id = new_lex_node(kID);
  read_token();
  if (last_token == kLP) {
    Astnode *p_lp = new_lex_node(kLP);
    read_token();
    if (last_token == kRP) {
      Astnode *p_rp = new_lex_node(kRP);
      read_token();
      Astnode *p = new_syntax_node("Exp", 3);
      build3(p, p_id, p_lp, p_rp);
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
    Astnode *p_rp = new_lex_node(kRP);
    read_token();
    Astnode *p = new_syntax_node("Exp", 4);
    build4(p, p_id, p_lp, p_args, p_rp);
    return p;
  }
  Astnode *p = new_syntax_node("Exp", 1);
  build1(p, p_id);
  return p;
}

Astnode* parser_uop_exp() {
  int cur_lineno = lineno;
  Astnode *p_uop = new_lex_node(last_token);
  read_token();
  Astnode *p_exp = parser_primary();
  if (p_exp == NULL) {
    // TODO
    free1(p_uop);
    return NULL;
  }
  Astnode *p = new_syntax_node("Exp", 2);
  build2(p, p_uop, p_exp);
  return p;
}

Astnode* parser_op_rhs(int lst_prec, Astnode *p_lhs) {
  for (;;) {
    int cur_prec = get_token_precedence();
    if (cur_prec < lst_prec) return lhs;
    Token op = last_token;
    // need to save: RELOP
    // special case: LB, DOT
    read_token();
    if (op == kDOT) {
      if (last_token != kID) return 1;
      lhs = 0;
      continue;
    }
    if (op == kLB) {
      if (!parser_exp()) return 1;
      if (last_token != kRB) return 1;
      read_token();
      lhs = 0;
      continue;
    }
    int rhs = parser_primary();
    if (rhs) return 1;
    int nxt_prec = get_token_precedence();
    if (cur_prec < nxt_prec) {
      rhs = parser_op_rhs(cur_prec, rhs);
    }
    lhs = 0;  // merge lhs, op, rhs
  }
}

Astnode* parser_primary() {
  switch (last_token) {
    case kINT: case kFLOAT: return parser_val_exp();
    case kID: return parser_id_exp(); 
    case kLP: return parser_paren_exp(); 
    case kMINUS: case kNOT: return parser_uop_exp();
    default: return 1;
  }
}

Astnode* parser_exp() {
  Astnode *p_lhs = parser_primary();
  if (p_lhs == NULL) {
    //TODO
    return NULL:
  }
  return parser_op_rhs(0, lhs);
}

int parser_vardec() {
  if (last_token != kID)
    return 1;
  read_token();
  while (last_token == kLB) {
    read_token();
    if (last_token != kINT)
      return 1;
    read_token();
    if (last_token != kRB)
      return 1;
    read_token();
  }
  return 0;
}

int parser_dec() {
  if (!parser_vardec())
    return 1;
  if (last_token == kASSIGNOP) {
    read_token();
    if (!parser_exp())
      return 1;
  }
  return 0;
}

int parser_def() {
  if (!parser_specifier())
    return 1;
  if (!parser_declist())
    return 1;
  return 0;
}

int parser_extdef() {
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

int parser_paramdec() {
  if (!parser_specifier())
    return 1;
  if (!parser_vardec())
    return 1;
  return 0;
}

int parser_specifier() {
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

int parser_compst() {
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

int parser_stmt() {
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
  if (!parser_exp())
    return 1;
  if (last_token == kCOMMA) {
    read_token();
    if (!parser_args())
      return 1;
  }
  return 0;
}

int parser_varlist() {
  if (!parser_paramdec())
    return 1;
  if (last_token == kCOMMA) {
    read_token();
    if (!parser_varlist())
      return 1;
  }
  return 0;
}

int parser_declist() {
  if (!parser_dec())
    return 1;
  if (last_token == kCOMMA) {
    read_token();
    if (!parser_declist())
      return 1;
  }
  return 0;
}

int parser_deflist() {
  if (last_token == kTYPE || last_token == kSTRUCT) {
    if (!parser_def())
      return 1;
    if (!parser_deflist())
      return 1;
  }
  return 0; 
}

int parser_extdeclist() {
  if (!parser_vardec())
    return 1;
  if (last_token == kCOMMA) {
    read_token();
    if (!parser_extdeclist())
        return 1;
  }
}

int parser_extdeflist() {
  if (last_token == kTYPE || last_token == kSTRUCT) {
    if (!parser_extdef())
      return 1;
    if (!parser_extdeflist())
      return 1;
  }
  return 0;
}

int parser_stmtlist() {
  if (last_token != kRC) {
    if (!parser_stmt())
      return 1;
    if (!parser_stmtlist())
      return 1;
  }
  return 0;
}

int parser_program() {
  read_token();
  if (!parser_extdeflist())
    return 1;
  return 0;
}

#undef new_lex_node
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

