//
//  helper.h  misc functions for debug and linked list maintenance
//  Compiler
//
//

#ifndef helper_h
#define helper_h

#include <stdbool.h>
#include "ast.h"
#include "parser.h"

//structure to hold symbols
typedef struct  symStruct{
    char* name;
    //Token type; // this will always be kwINT but theoreticaly it could be other types
    struct symStruct *previous;
    struct symStruct *next;
} symNode;

//structure to hold linked list of function definition names and arg count
typedef struct funcStruct {
    char* name;
    int nargs;
    symNode* symbolTable;
    struct funcStruct *next;
} funcNode;

typedef enum {global, local, temp, parameter, constant, functionresult, unknown} idType;

//structure to hold linked list of symbols on the stack
typedef struct LocalsStruct{
    char* name;
    idType type;
    int offset;
    struct LocalsStruct *next;
} LocalsNode;

//LocalsNode* LocalsList

//extern funcNode* funcList      ;
//extern symNode*  symbolTable   ;
//extern symNode*  symListHead   ;
//extern symNode*  symListTail   ;



extern int line_num;    // defined in scanner. used here to print error messages
extern int dbg;         // defined in driver.c
extern char* GActiveFunctionName; //defined in mipsgen
extern char* GCallingFunctionName; //defined in mipsgen
extern int labelCnt; // defined in codegen

const char* getTokenName(Token T);
void error_message(char* message) ;
void addFuncName(char* fname);
char* addSymbolName(char* fname);
void UpdateFuncListArgs(char* fname, int args);
void moveLocalSymbols(char* fname);
void printSymList(symNode *list);
void printFuncList(void);
void CheckFuncDeclaration(char* fname,int args);
bool isRELOP(Token T);
bool isLOP(Token T);
char* symTabLookup(char* symbol);
bool isGlobalSymbol (char* name);
NodeType getNodeType(Token T);
const char* getTypeName(NodeType T);
char* funTabLookup(char* symbol);
int GetArgsCnt(char* fname);
void freeMarker(void);
void prt(char* text);
void prtc(char c) ;
void prtln(char* text) ;
void comment(char* text) ;
int idLookup(char* symbol);
void putParmsInLocalsList(char* id);
void putVariableInLocalsList(char* id, idType type);
symNode* getLocalVariablesPtrForFunction(char* fname);
bool isLocalSymbol (char* name);
void deallocateLocalsList(void);
idType getIdType(char* id) ;
int getIdOffset(char* id) ;
bool inLocalList(char* id);
int getParmCount(char* fname);
char* newLabel(void);


#endif /* helper_h */
