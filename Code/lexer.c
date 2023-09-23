#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"

char token_str[MAX_TOKEN_LEN + 1];
float token_float_val;
int token_int_val;
int lineno = 1;
int error_cnt = 0;
char *token_name[] = {
  "EOF",
  "INT",
  "FLOAT",
  "ID",
  "SEMI",
  "COMMA",
  "ASSIGNOP",
  "RELOP",
  "PLUS",
  "MINUS",
  "STAR",
  "DIV",
  "AND",
  "OR",
  "DOT",
  "NOT",
  "TYPE",
  "LP",
  "RP",
  "LB",
  "RB",
  "LC",
  "RC",
  "STRUCT",
  "RETURN",
  "IF",
  "ELSE",
  "WHILE",
};

#define check_token_strlen(i) do { \
      if (i >= MAX_TOKEN_LEN) { \
        ++error_cnt; \
        printf("Error type A at Line %d: Too long for a token.\n", lineno); \
        return get_token(); \
      } \
    } while (0)

#define check_suffix() do { \
      if (isalpha(last_char) || last_char == '_') { \
        do read_char(); while (isalnum(last_char) || last_char == '_'); \
        ++error_cnt; \
        printf("Error type A at Line %d: Invalid suffix.\n", lineno); \
        return get_token(); \
      } \
    } while (0)

static char last_char = ' ';
void read_char() {
  last_char = getchar();
  if (last_char == '\n') ++lineno;
}

bool is8digit(char c) { return '0' <= c && c <= '7'; }
int decode16(char c) {
  if (isdigit(c)) return c - '0';
  if (isupper(c)) return c - 'A' + 10;
  else return c - 'a' + 10;
}

Token get_token() {
  while (isspace(last_char)) read_char();
  switch (last_char) {
    case '!': {
      read_char();
      if (last_char == '=') {
        token_str[0] = '!';
        token_str[1] = '=';
        token_str[2] = '\0';
        return kRELOP;
      }
      return kNOT;
    }
    case '&': {
      read_char();
      if (last_char == '&') {
        read_char();
        return kAND;
      }
      ++error_cnt;
      printf("Error type A at Line %d: Expected \"&&\".\n", lineno);
      return get_token();
    }
    case '(': { read_char(); return kLP; }
    case ')': { read_char(); return kRP; }
    case '*': { read_char(); return kSTAR; }
    case '+': { read_char(); return kPLUS; }
    case ',': { read_char(); return kCOMMA; }
    case '-': { read_char(); return kMINUS; }
    case '.': { read_char(); return kDOT; }  /// TODO: in case of float
    case '/': {
      read_char();
      if (last_char == '/') {
        do {
          read_char();
        } while (last_char != '\n' && last_char != EOF);
        return get_token();
      } else if (last_char == '*') {
        do {
          read_char();
          if (last_char == '*') {
            read_char();
            if (last_char == '/') {
              read_char();
              return get_token();
            }
          }
        } while (last_char != EOF);
        ++error_cnt;
        printf("Error type A at Line %d: Missing \"*/\".\n", lineno);
        return get_token();
      }
      return kDIV; 
    }
    case '0': {  // TODO: details
      read_char();
      if (is8digit(last_char)) {  // TODO: in case of float
        token_int_val = 0;
        do {
          token_int_val = 8 * token_int_val + last_char - '0';
          read_char();
        } while (is8digit(last_char));
        check_suffix();
        return kINT;
      } else if (last_char == 'x' || last_char == 'X') {
        read_char();
        if (isxdigit(last_char)) {
          token_int_val = 0;
          do {
            token_int_val = 16 * token_int_val + decode16(last_char);
            read_char();
          } while (isxdigit(last_char));
          check_suffix();
          return kINT;
        } else {
          ++error_cnt;
          printf("Error type A at Line %d: Expected a hex number.\n", lineno);
          return get_token();
        }
      } else {
        check_suffix();
        token_int_val = 0;
        return kINT;
      }
    }
    case '1': case '2': case '3':
    case '4': case '5': case '6':
    case '7': case '8': case '9': {
      // processing int
      int i = 0;
      token_int_val = 0;
      do {
        if (i < MAX_TOKEN_LEN) token_str[i++] = last_char;
        token_int_val = 10 * token_int_val + last_char - '0';
        read_char();
      } while (isdigit(last_char));
      if (last_char != 'e' && last_char != 'E') check_suffix();
      if (last_char != 'e' && last_char != 'E' && last_char != '.') return kINT;
      // processing float TODO: eE
      if (i < MAX_TOKEN_LEN) token_str[i++] = '.';
      read_char();
      while (isdigit(last_char)) {
        if (i < MAX_TOKEN_LEN) token_str[i++] = last_char;
        read_char();
      }
      if (last_char != 'e' && last_char != 'E') check_suffix();
      check_token_strlen(i);
      token_str[i] = '\0';
      token_float_val = strtod(token_str, NULL);
      return kFLOAT;
    }
    case ';': { read_char(); return kSEMI; }
    case '<': {
      read_char();
      if (last_char == '=') {
        read_char();
        token_str[0] = '<';
        token_str[1] = '=';
        token_str[2] = '\0';
        return kRELOP;
      }
      token_str[0] = '<';
      token_str[1] = '\0';
      return kRELOP;
    }
    case '=': {
      read_char();
      if (last_char == '=') {
        read_char();
        token_str[0] = '=';
        token_str[1] = '=';
        token_str[2] = '\0';
        return kRELOP;
      }
      token_str[0] = '=';
      token_str[1] = '\0';
      return kASSIGNOP;
    }
    case '>': {
      read_char();
      if (last_char == '=') {
        read_char();
        token_str[0] = '>';
        token_str[1] = '=';
        token_str[2] = '\0';
        return kRELOP;
      }
      token_str[0] = '>';
      token_str[1] = '\0';
      return kRELOP;  
    }
    case '[': { read_char(); return kLB; }
    case ']': { read_char(); return kRB; }
    case '{': { read_char(); return kLC; }
    case '|': {
      read_char();
      if (last_char == '|') {
        read_char();
        return kOR;
      }
      ++error_cnt;
      printf("Error type A at Line %d: Expected \"||\".\n", lineno);
      return get_token();
    }
    case '}': { read_char(); return kRC; }
    default: {
      if (isalpha(last_char) || last_char == '_') {
        int i = 0;
        do {
          if (i < MAX_TOKEN_LEN) token_str[i++] = last_char;
          read_char();
        } while (isalnum(last_char) || last_char == '_');
        check_token_strlen(i);
        token_str[i] = '\0';
        if (!strcmp(token_str, "int")) return kTYPE;  
        if (!strcmp(token_str, "double")) return kTYPE;
        if (!strcmp(token_str, "struct")) return kSTRUCT;
        if (!strcmp(token_str, "return")) return kRETURN;
        if (!strcmp(token_str, "if")) return kIF;
        if (!strcmp(token_str, "else")) return kELSE;
        if (!strcmp(token_str, "while")) return kWHILE;
        return kID;
      } else {
        if (last_char == EOF) return kEOF;
        ++error_cnt;
        printf("Error type A at Line %d: Mysterious character '%c'.\n", lineno, last_char);
        read_char();
        return get_token();
      }
    }
  }
}

