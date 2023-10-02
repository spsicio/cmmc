#ifndef CMMC_LEXER_H
#define CMMC_LEXER_H

#define MAX_TOKEN_LEN (32)

#include <stdio.h>

extern FILE *fp;
extern char token_str[];
extern float token_float_val;
extern int token_int_val;
extern int lineno;
extern int error_cnt;
extern char* token_name[];

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
  kNAT,  // not a type
} Token;

Token get_token();

#endif  // CMCC_LEXER_H

