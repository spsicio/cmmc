#ifndef CMMC_LEXER_H
#define CMMC_LEXER_H

#include <stdbool.h>
#define MAX_TOKEN_LEN (255)

extern char token_str[];
extern float token_float_val;
extern int token_int_val;
extern int lineno;
extern bool have_error;

typedef enum {
  kEOF,
  kINT,
  kFLOAT,
  kID,
  kSEMI,
  kCOMMA,
  kASSIGNOP,
  kRELOP,
  kPLUS,
  kMINUS,
  kSTAR,
  kDIV,
  kAND,
  kOR,
  kDOT,
  kNOT,
  kTYPE,
  kLP,
  kRP,
  kLB,
  kRB,
  kLC,
  kRC,
  kSTRUCT,
  kRETURN,
  kIF,
  kELSE,
  kWHILE, 
} Token;

Token get_token();

#endif  // CMCC_LEXER_H

