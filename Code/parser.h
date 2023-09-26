#ifndef CMMC_PARSER_H
#define CMMC_PARSER_H

int parser_primary();
int parser_op_rhs(int, int);
int parser_paren_exp();
int parser_val_exp();
int parser_id_exp();
int parser_neg_exp();
int parser_not_exp();

int parser_exp();         // 表達式
int parser_vardec();      // 變量名
int parser_dec();         // 可賦值的變量名
int parser_def();         // 定義變量
int parser_extdef();      // 定義變量，結構體，函數；
int parser_paramdec();    // 標識符＋變量名
int parser_specifier();   // 標識符
int parser_compst();      // 語句塊
int parser_stmt();        // 語句

int parser_args();
int parser_varlist();
int parser_declist();
int parser_deflist();
int parser_extdeclist();
int parser_extdeflist();
int parser_stmtlist();
int parser_program();

#endif  // CMMC_PARSER_H

