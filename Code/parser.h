#ifndef CMMC_PARSER_H
#define CMMC_PARSER_H

#include <stdbool.h>
#include "ast.h"

extern Token last_token;
void read_token();

// EXPRESSION
Astnode* parser_val_exp();
Astnode* parser_id_exp();
Astnode* parser_uop_exp();
Astnode* parser_paren_exp();
Astnode* parser_primary();
Astnode* parser_op_rhs(int, Astnode*);
Astnode* parser_exp();

// STATEMENT
Astnode* parser_compst();
Astnode* parser_return_stmt();
Astnode* parser_if_stmt();
Astnode* parser_while_stmt();
Astnode* parser_exp_stmt();
Astnode* parser_stmt();
Astnode* parser_stmtlist(Astnode*, int);

// COMPONENT
Astnode* parser_args();
Astnode* parser_paramdec();
Astnode* parser_varlist();
Astnode* parser_fundec(Astnode*, int);
Astnode* parser_vardec(Astnode*, int);
Astnode* parser_extdeclist(Astnode*, int);
Astnode* parser_dec();
Astnode* parser_declist();
Astnode* parser_specifier();

// DEFINITION
Astnode* parser_def();
Astnode* parser_deflist(Astnode*, int, bool);
Astnode* parser_extdef();
Astnode* parser_extdeflist();
Astnode* parser_program();

#endif  // CMMC_PARSER_H

