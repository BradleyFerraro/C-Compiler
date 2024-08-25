//
//  parser.h
//  Compiler
//
//

#ifndef parser_h
#define parser_h

#include <stdbool.h>
#include "scanner.h"
#include "ast.h"


extern char* lexeme; // defined in parser
extern int line_num; // defined in parser

extern int chk_decl_flag;
extern int print_ast_flag;
extern int gen_code_flag;


extern Token get_token(void);
void error_message();
void parse_function();
void prog(void);
ASTNode* var_decl(void);
void id_list(void);
void type();
ASTNode* func_defn(char* fname);
ASTNode* opt_formals(int* count);
ASTNode* formals(int* count);
void opt_var_decls(void);
ASTNode* opt_stmt_list();
ASTNode* stmt();
ASTNode* if_stmt();
ASTNode* while_stmt();
ASTNode* return_stmt();
ASTNode* assg_stmt(char* ID);
ASTNode* fn_call(char* fname);
ASTNode* opt_expr_list(int* count);
ASTNode* expr_list(int* count);
ASTNode* Expression(void);
ASTNode* Primary(void);
ASTNode* UnaryExpr(void);
ASTNode* MulExpr(void);
ASTNode* AddExpr(void);
ASTNode* RelationalExpr(void);
ASTNode* EqualityExpr(void);
ASTNode* AndExpr(void);
ASTNode* OrExpr(void);
//ASTNode* bool_exp();
ASTNode* arith_exp();
ASTNode* relop();
ASTNode*  term();
ASTNode*  factor();
ASTNode*  primary_exp();
ASTNode* Bexp(void);
ASTNode* Aexp(void);


#endif /* parser_h */
