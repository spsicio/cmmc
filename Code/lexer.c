#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "lexer.h"

FILE *fp;
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
      if (isalpha(last_char) || last_char == '_' || last_char == '.') { \
        read_suffix(); \
        ++error_cnt; \
        printf("Error type A at Line %d: Invalid suffix.\n", lineno); \
        return get_token(); \
      } \
    } while (0)

#define process_float() do { \
      if (last_char == 'e' || last_char == 'E') { \
        token_str_putc(&i, last_char); \
        read_char(); \
        if (last_char == '+' || last_char == '-') { \
          token_str_putc(&i, last_char); \
          read_char(); \
        } \
        if (isdigit(last_char)) { \
          do { \
            token_str_putc(&i, last_char); \
            read_char(); \
          } while (isdigit(last_char)); \
        } else { \
          read_suffix(); \
          ++error_cnt; \
          printf("Error type A at line %d: exponent has no digits.\n", lineno); \
          return get_token(); \
        } \
      } \
      check_suffix(); \
      check_token_strlen(i); \
      token_str[i] = '\0'; \
      token_float_val = strtod(token_str, NULL); \
      return kFLOAT; \
    } while (0);

static char last_char = ' ';
void read_char() {
  if (last_char == '\n') ++lineno;
  last_char = fgetc(fp);
}
void read_suffix() {
  while (isalnum(last_char) || last_char == '_' || last_char == '.') {
    read_char();
  }
}
void token_str_putc(int *i, char c) {
  if (*i < MAX_TOKEN_LEN) token_str[(*i)++] = c;
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
    case '.': {
      // processing dot or float
      read_char();
      if (!isdigit(last_char)) return kDOT;
      int i = 0;
      token_str_putc(&i, '.');
      do {
        token_str_putc(&i, last_char);
        read_char();
      } while (isdigit(last_char));
      process_float(); 
    }
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
    case '0': {
      read_char();
      if (isdigit(last_char)) {
        // processing oct or float
        int i = 0;
        bool isoct = 1;
        token_int_val = 0;
        do {
          token_str_putc(&i, last_char);
          token_int_val = 8 * token_int_val + last_char - '0';
          isoct &= is8digit(last_char);
          read_char();
        } while (isdigit(last_char));
        if (last_char != 'e' && last_char != 'E' && last_char != '.') {
          if (isoct) {
            check_suffix();
            return kINT;
          } else {
            read_suffix();
            ++error_cnt;
            printf("Error type A at Line %d: invalid oct number.\n", lineno);
            return get_token();
          }
        }
        if (last_char == '.') {
          token_str_putc(&i, '.'); 
          read_char();
          while (isdigit(last_char)) {
            token_str_putc(&i, '.');
            read_char();
          }
        }
        process_float();
      } else if (last_char == 'x' || last_char == 'X') {
        // processing hex
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
          check_suffix();
          ++error_cnt;
          printf("Error type A at Line %d: Expected a hex number.\n", lineno);
          return get_token();
        }
      } else {
        // processing zero or float
        if (last_char != 'e' && last_char != 'E' && last_char != '.') {
          check_suffix();
          token_int_val = 0;
          return kINT;
        }
        int i = 0;
        token_str_putc(&i, '0');
        if (last_char == '.') {
          token_str_putc(&i, '.');
          read_char();
          while (isdigit(last_char)) {
            token_str_putc(&i, last_char);
            read_char();
          };
        }
        process_float();
      }
    }
    case '1': case '2': case '3':
    case '4': case '5': case '6':
    case '7': case '8': case '9': {
      // processing int
      int i = 0;
      token_int_val = 0;
      do {
        token_str_putc(&i, last_char);
        token_int_val = 10 * token_int_val + last_char - '0';
        read_char();
      } while (isdigit(last_char));
      if (last_char != 'e' && last_char != 'E' && last_char != '.')  {
        check_suffix();
        return kINT;
      }
      // processing float
      if (last_char == '.') {
        token_str_putc(&i, '.');
        read_char();
        while (isdigit(last_char)) {
          token_str_putc(&i, last_char);
          read_char();
        }
      }
      process_float();
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
          token_str_putc(&i, last_char);
          read_char();
        } while (isalnum(last_char) || last_char == '_');
        check_token_strlen(i);
        token_str[i] = '\0';
        if (!strcmp(token_str, "int")) return kTYPE;  
        if (!strcmp(token_str, "float")) return kTYPE;
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

