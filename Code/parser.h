#ifndef CMMC_PARSER_H
#define CMMC_PARSER_H

#include "lexer.h"
#include "ast.h"

extern Token last_token;
void read_token();

Astnode* parser_primary();
Astnode* parser_op_rhs(int, Astnode*);
Astnode* parser_paren_exp();
Astnode* parser_val_exp();
Astnode* parser_id_exp();
Astnode* parser_uop_exp();

Astnode* parser_exp();         // 表達式
Astnode* parser_vardec();      // 變量名
Astnode* parser_dec();         // 可賦值的變量名
Astnode* parser_def();         // 定義變量
Astnode* parser_extdef();      // 定義變量，結構體，函數；
Astnode* parser_fundec();      // 函數名及參數列表
Astnode* parser_paramdec();    // 標識符＋變量名
Astnode* parser_specifier();   // 標識符
Astnode* parser_compst();      // 語句塊
Astnode* parser_stmt();        // 語句

Astnode* parser_args();
Astnode* parser_varlist();
Astnode* parser_declist();
Astnode* parser_deflist();
Astnode* parser_extdeclist();
Astnode* parser_extdeflist();
Astnode* parser_stmtlist();
Astnode* parser_program();

#endif  // CMMC_PARSER_H

