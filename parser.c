// Bradley Ferraro 
// Assignment 2 Milestone 1
// CS 453
/* 
compile.c is a program that implements 
a function parse() that behaves as follows:

It uses the scanner from Assignment 1 Milestone 
1 to tokenize the input, which is read from stdin.

It checks whether the input token sequence is a syntactically 
legal program with respect to the grammar rules for the G1 language.

If the input is syntactically correct, it then checks the semantics
of the g1 logic set. If they are correct, 
the parser exits silently with exit status 0.

If the input contains any syntax errors or semantic
errors, the parser prints out an appropriate error message
on stderr and exits with exit status 1. 
*/

/*
This is the makefile addition once codeGen.c and codeGen.h are in working order.

codeGen.o: codeGen.c codeGen.h
	gcc -c codeGen.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "scanner.h"
#include "ast.h"
#include "parser.h"
#include "helper.h"
#include "codegen.h"
#include "mipsgen.h"

Token currentToken;

// ast functions
ASTNode* newASTnode(NodeType type, char* name){
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    if (type == INTCONST) node->args=atoi(name);
    node->name = strdup(name);
    node->child0=NULL;
    node->child1=NULL;
    node->child2=NULL;
    node->code=NULL;
    return node;
}

typedef struct listStruct {
    ASTNode* node;
    struct listStruct* next;
} LinkedListNode;
  LinkedListNode *progHead=NULL, *progTail=NULL, *curProgNode=NULL, *prvProgNode=NULL;

// check if the token is what we expect and get the next token
void match(Token expectedToken) {
    if (currentToken == expectedToken) {
        currentToken = get_token();
    }
    else {
        char errmsg[255];
        sprintf(errmsg,"Token match failure: looking for:[%s] found:[%s]\n",getTokenName(expectedToken),getTokenName(currentToken));
        error_message(errmsg);
    }
}

// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------

int parse() {
    
    addFuncName("println");
    UpdateFuncListArgs("println", 1);

    currentToken = get_token();  // init token
    prog(); // parse a program
    freeMarker(); // if the global symbol table is empty it may contain a marker node

    if (gen_code_flag) {
        LinkedListNode *list=progHead;
        while (list != NULL) {
            genCode(list->node);
            list=list->next;
        }//while
//return 0;
        //printf("\n ---- start MIPS gen --\n");
        genGlobals();
        genPrintln();
        list=progHead;
        while (list != NULL) {
            genMips(list->node);
            list=list->next;
        }//while
    }//if
    return 0;
}

// match kwINT token
void type(void) {
    if (currentToken == kwINT) {
        match(kwINT);
    }
}


//prog :    func_defn   prog
//     |    var_decl   prog
//     |    ε
//(derivation)
//prog: type ID LPAREN   opt_formals   RPAREN   LBRACE   opt_var_decls   opt_stmt_list   RBRACE   prog
//      type ID SEMI   prog
//      type ID COMMA   id_list   SEMI   prog
//      ε



void  prog(void) {
    if (currentToken == kwINT) {
        match(kwINT);
        if (currentToken != ID) {error_message("ID not matched in prog().");}
        char* saveID = strdup(lexeme); // store the function or variable name
        match(ID); // match the function or variable name
        if (currentToken == LPAREN) { // this is a function definition
            curProgNode = malloc(sizeof(LinkedListNode));
            curProgNode->node = NULL;
            curProgNode->next=NULL;
            if (progHead == NULL) progHead = curProgNode; //first time
            curProgNode->node=func_defn(saveID);
            if (prvProgNode != NULL) prvProgNode->next = curProgNode;
            prvProgNode=curProgNode;
            prog();
        }else
            if (currentToken == SEMI) {  // This is a single variable
            addSymbolName(saveID); // store the id
            match(SEMI);
            prog();
        }else if (currentToken == COMMA) {  // This is multi variable
            addSymbolName(saveID); // store the id
            match(COMMA);
            id_list(); // process the variable list
            match(SEMI);
            prog();
        }else {
            error_message("unknown token in prog().");
        }
        free(saveID);
    } else if (currentToken != EOF) error_message("Missing kWINT");
    return; // token is not kwINT
} // end prog

//opt_var_decls :    ε
//              |    var_decl   opt_var_decls
void  opt_var_decls(void) {
    while (currentToken == kwINT) {
        match(kwINT);
        if (currentToken != ID) error_message("Syntax error in variable declaration");
        id_list();
        match(SEMI);
    }
}

// id_list:     ID  |  ID   COMMA   id_list
void  id_list() {
    if (currentToken == ID) {
        addSymbolName(lexeme);
        match(ID);
        if (currentToken == COMMA) {
            match(COMMA);
            id_list();
        }
        
        /*
        // added recently
        else if (currentToken == opASSG) {
            match(opASSG);
            match(INTCON);
        }
        // might be wrong
        */
        
    } // token is not ID so fall out
} //id_list



// func_defn:    type   ID   LPAREN   opt_formals   RPAREN   LBRACE   opt_var_decls   opt_stmt_list   RBRACE
// we are here because we matched ID and the current token is "("
ASTNode* func_defn(char* fname) {
    extern symNode*  symbolTable; // declared in helper
    int args =0;
    ASTNode* node=newASTnode(FUNC_DEF,fname);
    if (isGlobalSymbol(fname) && chk_decl_flag) error_message("Function name is a global symbol");
    addFuncName(fname);  // this will abort the compiler if the name exists and checking is enabled
    match(LPAREN);
    node->child1 = opt_formals(&args);
    match(RPAREN);
    node->args = args;
    UpdateFuncListArgs(node->name, node->args);
    match(LBRACE);
    opt_var_decls();
    node->child0 = opt_stmt_list();
    match(RBRACE);
    moveLocalSymbols(fname);
    if (dbg) {printSymList(symbolTable); printFuncList();}
    if (print_ast_flag == 1) {
        print_ast(node);
    }
    return node;
}

//opt_formals    :    ε
//                  formals
ASTNode* opt_formals(int* count) {
    if (currentToken == kwINT) {
        return formals(count);
    }
    return NULL;
}

//formals:    type ID   COMMA   formals
//       |    type ID
ASTNode* formals(int* count) {
    match(kwINT);
    ASTNode* node=newASTnode(IDENTIFIER,addSymbolName(strdup(lexeme)));
    match(ID);
    if (currentToken == COMMA) {
        match(COMMA);
        (*count)++;
        node->child1 = formals(count);
    }else if (currentToken == RPAREN) {
        (*count)++;
    }
    return node;
}

//opt_stmt_list:    stmt   opt_stmt_list |   ε
ASTNode* opt_stmt_list(void) {
    ASTNode *head=NULL, *tail=NULL, *previous=NULL;
    while (currentToken == ID || currentToken == kwIF || currentToken == kwRETURN ||
        currentToken == LBRACE || currentToken == SEMI || currentToken == kwWHILE) {
        tail = newASTnode(STMT_LIST, "");
        tail->child0=stmt();
        if (head == NULL) head=tail;
        else previous->child1=tail;
        previous=tail;
       }
   return(head);
} //opt_stmt_list

        
            
 //stmt:     fn_call   SEMI
//     |    while_stmt
//     |    if_stmt
//     |    assg_stmt
//     |    return_stmt
//     |    LBRACE   opt_stmt_list   RBRACE
//     |    SEMI

ASTNode* stmt(void) {
    ASTNode* node;
    if (currentToken == ID) { // assg_stmt | fn_call
        char* saveID = strdup(lexeme);
        match(ID);
        if (currentToken == LPAREN) { //fn_call
            node= fn_call(funTabLookup(saveID));
            match(SEMI);
        }
        else { //assg_stmt
            node= assg_stmt(symTabLookup(saveID));
        }
        free(saveID);
    } else if (currentToken == kwWHILE) {
        node=while_stmt();
    } else if (currentToken == kwIF) {
        node=if_stmt();
    } else if (currentToken == kwRETURN) {
        node=return_stmt();
    } else if (currentToken == LBRACE) {
        match(LBRACE);
        node=opt_stmt_list();
        match(RBRACE);
    } else {
        match(SEMI);
        node=NULL;
    }
    return node;
}

ASTNode* if_stmt(void) {
    ASTNode* node=newASTnode(IF,lexeme);
    match(kwIF);
    match(LPAREN);
    node->child0 = Expression();
    match(RPAREN);
    node->child1 = stmt();
    if (currentToken == kwELSE) {
        match(kwELSE);
        node->child2 = stmt();
    }
    return node;
}


ASTNode* while_stmt(void) {
    ASTNode* node=newASTnode(WHILE,lexeme);
    match(kwWHILE);
    match(LPAREN);
    node->child0 = Expression();
    match(RPAREN);
    node->child1 = stmt();
    return node;
}


ASTNode* return_stmt(void) {
    ASTNode* node=newASTnode(RETURN,lexeme);
    match(kwRETURN);
    if (currentToken == SEMI) {
        match(SEMI);
        return node;
    }
    node->child0 = arith_exp();
    match(SEMI);
    return node;
}

// assg_stmt    :    ID   opASSG   arith_exp   SEMI
ASTNode* assg_stmt(char* variable) {
    ASTNode* node=newASTnode(ASSG,lexeme);
    node->child0 = newASTnode(IDENTIFIER,strdup(symTabLookup(variable)));
    match(opASSG);
    node->child1 = arith_exp();
    if (node->child1 ==NULL) error_message("Missing oporand in assignment statement.");
    match(SEMI);
    return node;
}

//fn_call    :    ID   LPAREN   opt_expr_list   RPAREN
ASTNode* fn_call(char* fname) {
    int args=0;
    match(LPAREN);
    ASTNode* node=newASTnode(FUNC_CALL,fname);
    node->args = GetArgsCnt(fname);
    node->child0 = opt_expr_list(&args);
    match(RPAREN);
    CheckFuncDeclaration(fname,args); //check that the function was properly declared
    return node;
}


//opt_expr_list    :    ε
//                  |    expr_list
ASTNode* opt_expr_list(int* count) {
    if (currentToken == RPAREN) { // end of expression list
        return NULL;
    }
    return expr_list(count);
}

//expr_list    :    arith_exp   COMMA   expr_list
//              |    arith_exp
ASTNode* expr_list(int* count) {
    ASTNode* node=newASTnode(EXPR_LIST,"");
    (*count)++;
    node->child0 = arith_exp();
    if (currentToken == COMMA) {
        match(COMMA);
        if (currentToken == RPAREN) error_message("Missing identifier in expression list.");
        node->child1 = expr_list(count);

    }
    return node;
}



// -------------------------------------------------------
// -------------------------------------------------------
// -------------------------------------------------------



ASTNode* Primary(void) {
    ASTNode* node = NULL;
    char* identifier;
    if (dbg) printf("->Primary\n");
    if (currentToken == LPAREN) {
        match(LPAREN);
        node = Expression();
        match(RPAREN);
    }
    // how to organize primary to handle function calls and parens at every possibility...?
    else if (currentToken == ID) {
        identifier = strdup(lexeme);
        match(ID);
        if (currentToken == LPAREN) {
            funTabLookup(identifier);
            node=fn_call(identifier);
        } else { // its a variable
            symTabLookup(identifier);
            node=newASTnode(IDENTIFIER,identifier);
        }
        free(identifier);
    }
    else if (currentToken == INTCON) {
        node=newASTnode(INTCONST,lexeme);
        match(INTCON);
    }
    return node;
}


ASTNode* UnaryExpr(void) {
    if (dbg) printf("->UnaryExpr\n");
     if (currentToken == opSUB) {
        ASTNode* node = newASTnode(UMINUS, lexeme);
        match(opSUB);
        node->child0 = UnaryExpr();
        if (node->child0 == NULL) {
            error_message("Syntax error in unary expression.");
        }
        return node;
    } else {
        return Primary();
    }
}

ASTNode* MulExpr(void) {
    if (dbg) printf("->MulExpr\n");
    ASTNode* node = UnaryExpr();
    while (currentToken == opMUL || currentToken == opDIV) {
        ASTNode* newNode = newASTnode(getNodeType(currentToken), lexeme);
        match(currentToken);
        newNode->child0 = node;
        newNode->child1 = UnaryExpr();
        if (newNode->child1 == NULL) {
            error_message("Syntax error in mul/div expression.");
        }
        node = newNode;
    }
    return node;
}

ASTNode* AddExpr(void) {
    if (dbg) printf("->AddExpr\n");
    ASTNode* node = MulExpr();
    while (currentToken == opADD || currentToken == opSUB) {
        ASTNode* newNode = newASTnode(getNodeType(currentToken), lexeme);
        match(currentToken);
        newNode->child0 = node;
        newNode->child1 = MulExpr();
        if (newNode->child1 == NULL) {
            error_message("Syntax error in add/sub expression.");
        }
        node = newNode;
    }
    return node;
}

ASTNode* RelationalExpr(void) {
    if (dbg) printf("->RelationalExpr\n");
    ASTNode* node = AddExpr();
    while (isRELOP(currentToken)) {
        ASTNode* newNode = newASTnode(getNodeType(currentToken), lexeme);
        match(currentToken);
        newNode->child0 = node;
        newNode->child1 = AddExpr();
        if (newNode->child1 == NULL) {
            error_message("Syntax error in rel expression.");
        }
        node = newNode;
    }
    return node;
}

ASTNode* EqualityExpr(void) {
    if (dbg) printf("->EqualityExpr\n");
    ASTNode* node = RelationalExpr();
    while (currentToken == opEQ || currentToken == opNE) {
        ASTNode* newNode = newASTnode(getNodeType(currentToken), lexeme);
        match(currentToken);
        newNode->child0 = node;
        newNode->child1 = RelationalExpr();
        if (newNode->child1 == NULL) {
            error_message("Syntax error in eq expression.");
        }
        node = newNode;
    }
    return node;
}

ASTNode* AndExpr(void) {
    if (dbg) printf("->AndExpr\n");
    ASTNode* node = EqualityExpr();
    while (currentToken == opAND) {
        ASTNode* newNode = newASTnode(AND, lexeme);
        match(opAND);
        newNode->child0 = node;
        newNode->child1 = EqualityExpr();
        if (newNode->child1 == NULL) {
            error_message("Syntax error in log AND expression.");
        }
        node = newNode;
    }
    return node;
}

ASTNode* OrExpr(void) {
    if (dbg) printf("->OrExpr\n");
    ASTNode* node = AndExpr();
    while (currentToken == opOR) {
        ASTNode* newNode = newASTnode(OR, lexeme);
        match(opOR);
        newNode->child0 = node;
        newNode->child1 = AndExpr();
        if (newNode->child1 == NULL) {
            error_message("Syntax error in log OR expression.");
        }
        node = newNode;
    }
    return node;
}

ASTNode* Expression(void) {
    if (dbg) printf("->Expression\n");
    return OrExpr();
}





// -------------------------------------------------------
// -------------------------------------------------------
// -------------------------------------------------------


// An Unambiguous Grammar for arithmetic Expressions
// precedence - The distinction among expressions, terms and factors enforces the correct
// grouping of operators at different levels of precedence
// associativity - to make it left associative move the paths that lead most quickly to a terminal right
// E -> T+E T-E T right associative if you change it, it;s left recursive 
// T -> F*T F/T F
// F-> (E) N -F
// N-> ID INTCON FNcall
 
//P-> ID INTCON FNcall - {terminals}
 ASTNode*  primary_exp(void) {
     char* identifier;
     ASTNode* node= NULL;
     if (currentToken == ID) {
         identifier = strdup(lexeme);
         match(ID);
         if (currentToken == LPAREN) {
             funTabLookup(identifier);
             node=fn_call(identifier);
         } else { // its a variable
             symTabLookup(identifier);
             node=newASTnode(IDENTIFIER,identifier);
         }
         free(identifier);
     }
     else if (currentToken == INTCON) {
        node=newASTnode(INTCONST,lexeme);
        match(INTCON);

     }
     return node;
 }

// F-> (E) -F P
 ASTNode* factor(void) {
      ASTNode *newNode;
      if (currentToken == LPAREN) {
         match(LPAREN);
         newNode=arith_exp();
         match(RPAREN);
     }else if (currentToken == opSUB) {
        newNode=newASTnode(UMINUS,lexeme);
        match(opSUB);
        newNode->child0= factor();
        if (newNode->child0==NULL) error_message("missing unary minus operand.");
     }else {
         newNode=primary_exp();
     }
     return newNode;
 }

// T' -> ~ | * FT'
ASTNode* Tprime(ASTNode* node) {
    if ((currentToken == opMUL) || (currentToken == opDIV)) {
        ASTNode* newNode=newASTnode(getNodeType(currentToken),lexeme);
        match(currentToken);
        newNode->child0 = node; //factor();
        newNode->child1 = factor(); //Tprime(node);
        if (node==NULL) error_message("missing mul or div operands.");
        if (newNode->child1==NULL) error_message("missing mul or div operands.");
        ASTNode* tPnode=Tprime(newNode);
        if (tPnode == NULL) return(newNode);
        else return(tPnode);
    } else return node;
}


// T -> FT'
ASTNode* term(void) {
    ASTNode *factorNode, *tPrimeNode;
    factorNode = factor();
    tPrimeNode= Tprime(factorNode);
    return tPrimeNode;
}


// E'->  + TE' | ~
ASTNode* eprime(ASTNode* node) {
    if ((currentToken == opADD) ||(currentToken == opSUB)){
        ASTNode* newNode=newASTnode(getNodeType(currentToken),lexeme);
        match(currentToken);
        newNode->child0 = node;
        newNode->child1 = term();
        if (node==NULL) error_message("missing add or sub operands.");
        if (newNode->child1==NULL) error_message("missing add or sub operands.");
        ASTNode* ePnode=eprime(newNode);
        if (ePnode == NULL) return(newNode);
        else return(ePnode);
    } else return node;
}

// E -> TE'
ASTNode* arith_exp(void) {
    ASTNode *termNode, *eprimeNode;
    termNode=term();
    eprimeNode=eprime(termNode);
    return(eprimeNode);
}








/*

ASTNode* relop() {
    ASTNode* astree = malloc(sizeof(ASTNode));
    if (currentToken == opEQ) {
        astree->type = EQ;
        match(opEQ);
        return astree;
    } else if (currentToken == opNE) {
        astree->type = NE;
        match(opNE);
        return astree;
    } else if (currentToken == opLE) {
        astree->type = LE;
        match(opLE);
        return astree;
    } else if (currentToken == opLT) {
        astree->type = LT;
        match(opLT);
        return astree;
    } else if (currentToken == opGE) {
        astree->type = GE;
        match(opGE);
        return astree;
    } else {
        astree->type = GT;
        match(opGT);
        return astree;
    }
}

ASTNode* arithop() {
    ASTNode* astree = malloc(sizeof(ASTNode));
    if (currentToken == opADD) {
        astree->type = ADD;
        match(opADD);
        return astree;
    } else if (currentToken == opSUB) {
        astree->type = SUB;
        match(opSUB);
        return astree;
    } else if (currentToken == opMUL) {
        astree->type = MUL;
        match(opMUL);
        return astree;
    } else  {
        // (currentToken == opDIV)
        astree->type = DIV;
        match(opDIV);
        return astree;
    }
}


ASTNode* logical_op() {
    ASTNode* astree = malloc(sizeof(ASTNode));
    if (currentToken == opAND) {
        astree->type = AND;
        match(opAND);
        return astree;
    } else {
        // (currentToken == opOR)
        astree->type = OR;
        match(opOR);
        return astree;
    }
}
*/
