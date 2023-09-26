#include <stdbool.h>
#include "lexer.h"
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
    case kDOT: return 7;
    case kLB: return 7;
    default: return -1;
  }
}

int parser_paren_exp() {
  read_token();
  if (!parser_exp())
    return 1;
  if (last_token != kRP)
    return 1;
  return 0;
}

int parser_val_exp() {
  read_token();
  return 0;
}

int parser_id_exp() {
  read_token();
  if (last_token == kLP) {
    read_token();
    if (last_token == kLP) {
      read_token();
      return 1;
    }
    if (!parser_args())
      return 1;
    if (last_token != kLP)
      return 1;
    read_token();
  }
  return 0;
}

int parser_neg_exp() {
  read_token();
  if (!parser_primary())
    return 1;
  return 0;
}

int parser_not_exp() {
  read_token();
  if (!parser_primary())
    return 1;
  return 0;
}

int parser_op_rhs(int lst_prec, int lhs) {
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

int parser_primary() {
  switch (last_token) {
    case kINT: case kFLOAT: return parser_val_exp();
    case kID: return parser_id_exp(); 
    case kLP: return parser_paren_exp(); 
    case kMINUS: return parser_neg_exp(); 
    case kNOT: return parser_not_exp();
    default: return 1;
  }
}

int parser_exp() {
  if (!parser_primary())
    return 1;
  return parser_op_rhs(0, 0);
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

int parser_args() {
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

