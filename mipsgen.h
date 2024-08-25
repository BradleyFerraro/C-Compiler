//
//  mipsgen.h
//  Compiler
//
//

#ifndef mipsgen_h
#define mipsgen_h

#include "ast.h"
char* genMips(ASTNode* node);
void genPrintln(void);
void genGlobals(void);

#endif /* mipsgen_h */
