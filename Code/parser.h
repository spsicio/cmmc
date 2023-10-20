#ifndef CMMC_PARSER_H
#define CMMC_PARSER_H

#include <stdbool.h>
#include "cst.h"

extern Token last_token;
void read_token();

// EXPRESSION
Cstnode* parser_val_exp();
Cstnode* parser_id_exp();
Cstnode* parser_uop_exp();
Cstnode* parser_paren_exp();
Cstnode* parser_primary();
Cstnode* parser_op_rhs(int, Cstnode*);
Cstnode* parser_exp();

// STATEMENT
Cstnode* parser_compst();
Cstnode* parser_return_stmt();
Cstnode* parser_if_stmt();
Cstnode* parser_while_stmt();
Cstnode* parser_exp_stmt();
Cstnode* parser_stmt();
Cstnode* parser_stmtlist(Cstnode*, int);

// COMPONENT
Cstnode* parser_args();
Cstnode* parser_paramdec();
Cstnode* parser_varlist();
Cstnode* parser_fundec(Cstnode*, int);
Cstnode* parser_vardec(Cstnode*, int);
Cstnode* parser_extdeclist(Cstnode*, int);
Cstnode* parser_dec();
Cstnode* parser_declist();
Cstnode* parser_specifier();

// DEFINITION
Cstnode* parser_def();
Cstnode* parser_deflist(Cstnode*, int, bool);
Cstnode* parser_extdef();
Cstnode* parser_extdeflist();
Cstnode* parser_program();

#endif  // CMMC_PARSER_H

