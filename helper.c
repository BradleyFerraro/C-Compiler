//
//  misc support functions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "helper.h"

//funcNode* funcList      =NULL;
//symNode*  symbolTable   =NULL;
//symNode*  symListHead   =NULL;
//symNode*  symListTail   =NULL;

funcNode* funcList      =NULL;
symNode*  symbolTable   =NULL;
symNode*  symListHead   =NULL;
symNode*  symListTail   =NULL;

LocalsNode* LocalsListHead  = NULL;
LocalsNode* LocalsListTail  = NULL;

char* marker="Scope change";
char errmsg[255]; // used in multiple functions


/*******************************************************************************************************************
 symbolTable is a pointer to the start of the global symbol table. When a funciton is defined we insert a marker into the end of that table,
 and point symListHead and symListTail at that marker. Local symbols are added to the end of the local symbol table. When a function
 definition ends, we unlink the local symbol table from the global symbol table and add a pointer to it in the functions table. We also
 point the end of the local symbol tabl to the global table so that during code generation we access local symbols first and then globals.
 */
void prt(char* text) {printf("%s",text);}
void prtc(char c) {printf("%c",c);}
void prtln(char* text) {printf("%s\n",text);}
void comment(char* text) {printf("# %s\n",text);}

// error message needs line numbers
void error_message(char* message) {
    fprintf(stderr, "Error line %3d  %s\n", line_num, message);
    //fprintf(stderr, "Error line %3d  \n", line_num);
    exit(1);
}

char* newLabel(void){
    char str[255];
    sprintf(str,":L%i",labelCnt++);
    return strdup(str);
}

void printSymList(symNode *list){
    printf("Symbol table\n");
    if (list == NULL) { printf(" >>Symbol table is empty.\n"); return; }
    while (list != NULL) { printf("%s\n",list->name); list=list->next;}
}

void printFuncList(void){
    funcNode  *list = funcList;
    printf("Function table\n");
    if (list == NULL) { printf(" >>Function table is empty.\n"); return; }
    while (list != NULL) {
        printf("%s (nargs:%d) \n",list->name,list->nargs);
        printSymList(list->symbolTable);
        list=list->next;}
}

// for debug returns the token string
const char* getTokenName(Token T){
   switch (T) {
       case  UNDEF:     return "UNDEF";
       case  ID:    return "ID";
       case  INTCON:    return "INTCON";
       case  LPAREN:    return "LPAREN";
       case  RPAREN:    return "RPAREN";
       case  LBRACE:    return "LBRACE";
       case  RBRACE:    return "RBRACE";
       case  COMMA:    return "COMMA";
       case  SEMI:    return "SEMI";
       case  kwINT:    return "kwINT";
       case  kwIF:    return "kwIF";
       case  kwELSE:    return "kwELSE";
       case  kwWHILE:    return "kwWHILE";
       case  kwRETURN: return "kwRETURN";
       case  opASSG:    return "opASSG";
       case  opADD:    return "opADD";
       case  opSUB:    return "opSUB";
       case  opMUL:    return "opMUL";
       case  opDIV:    return "opDIV";
       case  opEQ:    return "opEQ";
       case  opNE:    return "opNE";
       case  opGT:    return "opGT";
       case  opGE:    return "opGE";
       case  opLT:    return "opLT";
       case  opLE:    return "opLE";
       case  opAND:    return "opAND";
       case  opOR:    return "opOR";
       case  opNOT:    return "opNOT";
       default:    return "Bad token";
   }// switch
}// getTokenName

// for debug returns the type string
const char* getTypeName(NodeType T){
   switch (T) {
       case DUMMY:      return("DUMMY");
       case FUNC_DEF:   return("FUNC_DEF");
       case FUNC_CALL:  return("FUNC_CALL");
       case IF:         return("IF");
       case WHILE:      return("WHILE");
       case ASSG:       return("ASSG");
       case RETURN:     return("RETURN");
       case STMT_LIST:  return("STMT_LIST");
       case EXPR_LIST:  return("EXPR_LIST");
       case IDENTIFIER: return("IDENTIFIER");
       case INTCONST:   return("INTCONST");
       case EQ:         return("EQ");
       case NE:         return("NE");
       case LE:         return("LE");
       case LT:         return("LT");
       case GE:         return("GE");
       case GT:         return("GT");
       case ADD:        return("ADD");
       case SUB:        return("SUB");
       case MUL:        return("MUL");
       case DIV:        return("DIV");
       case UMINUS:     return("UMINUS");
       case AND:        return("AND");
       case OR:         return("OR");
       default:
           printf("Unknown node type in GetTypeName\n");
           break;
   }// switch
}// getTypeName


bool isRELOP(Token T){
    switch (T) {
        case  opEQ:    return true;
        case  opNE:    return true;
        case  opGT:    return true;
        case  opGE:    return true;
        case  opLT:    return true;
        case  opLE:    return true;
        default:       return false;
    }// switch
    return false;
 }


bool isLOP(Token T){
    switch (T) {
        case  opAND:    return true;
        case  opOR:     return true;
        default:        return false;
    }//
    return false;

 }
NodeType getNodeType(Token T){
    // get the node type for a givn token
    switch (T) {
        case  opEQ:    return EQ;
        case  opNE:    return NE;
        case  opGT:    return GT;
        case  opGE:    return GE;
        case  opLT:    return LT;
        case  opLE:    return LE;
        case  opAND:   return AND;
        case  opOR:    return OR;

        case  opADD:    return ADD;
        case  opSUB:    return SUB;
        case  opMUL:    return MUL;
        case  opDIV:    return DIV;

        default:    error_message("program logic error in getNodeType");
    }// switch
    return DUMMY; // will never get here
    
 }

// add formal parameters and their location on the stack to the locals list
void putParmsInLocalsList(char* id){
    char tmpStr[255];
    LocalsNode *newNode = malloc(sizeof(LocalsNode));  //malloc a new node
    newNode->name=strdup(id);
    newNode->type=parameter;
    newNode->next=NULL;
    if (LocalsListHead == NULL) { // first node
        LocalsListHead=newNode;
        LocalsListTail=newNode;
        newNode->offset=8;
    } else {
        newNode->offset =LocalsListTail->offset +4;
        LocalsListTail->next=newNode;
        LocalsListTail=newNode;
    }
    sprintf(tmpStr,"Put parameter %s in locals list at %d($fp)",newNode->name,newNode->offset);
    comment(tmpStr);
}

bool inLocalList(char* id) { // check if id is in the list
    LocalsNode *node = LocalsListHead;
    while (node != NULL){
        if (strcmp(node->name,id)==0) return true;
        node=node->next;
    }
    return false;
}


void putVariableInLocalsList(char* id, idType type){ //allocate space on the stack and add to the localsList table
    char tmpStr[255];
    // go to the last node on locallist, malloc a new node put it on the list
    // increment the stack pointer by 4 and store that address as an offset from $fp in the node associated to the id
    LocalsNode *newNode = malloc(sizeof(LocalsNode));  //malloc a new node
    newNode->name=strdup(id);
    newNode->type=type;
    newNode->next=NULL;
    if (LocalsListHead == NULL) { // first node means no parameters to the function
        LocalsListHead=newNode;
        LocalsListTail=newNode;
        newNode->offset=-4;
    } else { // not first node means previous locals or parameters
        if (LocalsListTail->type==parameter) newNode->offset=-4; else newNode->offset =LocalsListTail->offset -4;
        LocalsListTail->next=newNode;
        LocalsListTail=newNode;
    }
    sprintf(tmpStr,"allocate space for %s in locals list at offset %d($fp)",newNode->name,newNode->offset);
    comment(tmpStr);
    prtln("\taddi $sp, $sp, -4");
    prtln("");
    if (newNode->type == constant) {
        sprintf(tmpStr,"%08x",atoi(&id[1])); //8 characters with leading zeros
        //Load upper 16 bits of the constant
        prt("\tlui $t9, 0x");prtc(tmpStr[0]);prtc(tmpStr[1]);prtc(tmpStr[2]);prtc(tmpStr[3]);
        prtln("\t#Load upper 16 bits of the constant.");
        //load lower 16 bits of the constant (0x5678)
        prt("\tori $t9, $t9, 0x");prtc(tmpStr[4]);prtc(tmpStr[5]);prtc(tmpStr[6]);prtc(tmpStr[7]);
        prtln("\t#Load lower 16 bits of the constant.");

        // push it on the stack
        sprintf(tmpStr,"\tsw $t9, 0($sp) \t# save it on the stack");
        prtln(tmpStr);
        prtln("");
    }
}

// putTemporariesInLocalsList(atoi(p3), atoi(p4));


idType getIdType(char* id) { // get id type and add to locals if necessary
    LocalsNode *node = LocalsListHead;
    if (id[0] == '#') return constant;
    if (id[0] == '%') return functionresult; //stored in $v0
    while (node != NULL){
        if (strcmp(node->name,id)==0) return node->type;
        node = node->next;
    }
    // if we reach this point the id was not in the locals list so check if global.
    if (isGlobalSymbol(id)) return global;
    
    else printf("!error in getIdType on id: %s\n",id);
    return unknown;
}

int getIdOffset(char* id) { // get id offset, assumes id is in the locals list
    LocalsNode *node = LocalsListHead;
    while (node != NULL){
        if (strcmp(node->name,id)==0) return node->offset;
        node=node->next;
    }
    // if we reach this point the id was not in the locals list
    printf("!!! error in getIdOffset\n");
    return -999;
}

// free memory used by the locals list
void deallocateLocalsList(void){
    LocalsNode *currentNode =LocalsListHead;
    LocalsNode *tmpPtr;
    while (currentNode!= NULL){
        free(currentNode->name);
        tmpPtr=currentNode->next;
        free(currentNode);
        currentNode=tmpPtr;
    }
    LocalsListHead=NULL;
    LocalsListTail=NULL;
}


// add a function name to the list
// abort if chk_dcl_flag and this is a duplicate name
// if it is a duplicate and we are not checking, it will be added to the list anyway
void addFuncName(char* fname){
    funcNode *newNode, *currentNode, *previousNode = NULL;
    // special logic for first time in here
    if (funcList == NULL) {
        newNode = malloc(sizeof(funcNode));
        newNode->name = strdup(fname);
        newNode->next = NULL;
        funcList= newNode;
//        addSymbolName(marker); // add a marker  to the table to indicate the start of local symols
        return;
    }

    currentNode= funcList;
    while (currentNode != NULL) {
        if ((strcmp(currentNode->name,fname) == 0) && (chk_decl_flag)) {
            sprintf(errmsg,"Duplicate function name [%s]",fname);
            error_message(errmsg);
        }
        previousNode=currentNode;
        currentNode=currentNode->next;
    }
    newNode = malloc(sizeof(funcNode));
    previousNode->next=newNode;
    newNode->name = strdup(fname);
    newNode->next = NULL;
    addSymbolName(marker); // add a marker  to the table to indicate the start of local symols

}


int GetArgsCnt(char* fname){
  funcNode *currentNode = funcList;
  while (currentNode != NULL) {
        if (strcmp(currentNode->name,fname) == 0) {
            return currentNode->nargs ;
        }
        currentNode=currentNode->next;
    }
    if (chk_decl_flag){
        sprintf(errmsg,"*program error* Function name not found in GetArgsCnt[%s]\n",fname);
        error_message(errmsg);
    }
    return 0;
} //GetArgsCnt

    
void UpdateFuncListArgs(char* fname, int args){
    funcNode *currentNode = funcList;
    while (currentNode != NULL) {
        if (strcmp(currentNode->name,fname) == 0) {
            currentNode->nargs = args;
            return;
        }
        currentNode=currentNode->next;
    }
    if (chk_decl_flag){
        sprintf(errmsg,"Function name not found while updating args[%s]",fname);
        error_message(errmsg);
    }
} //UpdateFunctionArgs


// put pointer to symbol table in the the function 
void UpdateFuncListSymols(char*fname, symNode* ListPtr){
 funcNode* currentNode= funcList;
 while (currentNode != NULL) {
    if (strcmp(currentNode->name,fname) == 0) {
        currentNode->symbolTable=ListPtr;
        return;
    }
    currentNode=currentNode->next;
 }//while
 if (chk_decl_flag){
    sprintf(errmsg,"Function name not found while updating symbolTable[%s]",fname);
    error_message(errmsg);
 } //if
} //UpdateFunctionArgs

bool isLocalSymbol (char* name){
    symNode *symPtr=getLocalVariablesPtrForFunction(GActiveFunctionName); // get a pointer to the functions symboltable;
    if (symPtr != NULL) {
        while (symPtr != symbolTable) {// not wraped around to the global symbol table
            if (strcmp(name,symPtr->name) ==0) return true;
            symPtr=symPtr->next;
        }
    }
    return false;
}

bool isGlobalSymbol (char* name){
    symNode* currentNode= symbolTable;
    while (currentNode != NULL){
        if ((strcmp(currentNode->name,name) == 0)){ // found symbol name
            return(true);
        }
       if (strcmp(currentNode->name,marker) == 0) return false;
       currentNode=currentNode->next;
    }
    return false;
} //isGlobalSymbol

void CheckFuncDeclaration(char* fname, int args){
if (!chk_decl_flag) return;
funcNode *currentNode= funcList;
while (currentNode != NULL) {
    if ((strcmp(currentNode->name,fname) == 0)){ // found function name
        if (currentNode->nargs == args) return; // arg count matches, return
        sprintf(errmsg,"Function [%s] is called with different number of arguments than declaration.",fname);
        error_message(errmsg);
    }
    currentNode=currentNode->next;
 }
 printf(errmsg,"Function [%s] is called without being declared.",fname);
 error_message(errmsg);
} //UpdateFunctionArgs

symNode* getLocalVariablesPtrForFunction(char* fname){
    funcNode *currentNode= funcList;
    while (currentNode != NULL){
        if (strcmp(currentNode->name,fname) == 0) return currentNode->symbolTable; // found it
        currentNode=currentNode->next;
    } // while
    return NULL; // should never get here
}// funTabLookup

int getParmCount(char* fname){
    funcNode *currentNode= funcList;
    while (currentNode != NULL){
        if (strcmp(currentNode->name,fname) == 0) return currentNode->nargs; // found it
        currentNode=currentNode->next;
    } // while
    printf("!error in getParmsCount function :%s not found",fname);
    return 0; // should never get here
}// funTabLookup

char* funTabLookup(char* symbol){
    if (!chk_decl_flag) return symbol;
    funcNode *currentNode= funcList;
    while (currentNode != NULL){
        if (strcmp(currentNode->name,symbol) == 0) return symbol; // found it
        currentNode=currentNode->next;
    } // while
    sprintf(errmsg,"Undeclared function  [%s]. ",symbol);
    error_message(errmsg);
    return NULL; // should never get here
}// funTabLookup


// look up the symbol. Doesn't matter if it's global or local. abort if not found
char* symTabLookup(char* symbol){
    if (!chk_decl_flag) return symbol;
    symNode* currentNode= symbolTable;
    while (currentNode != NULL){
        if (strcmp(currentNode->name,symbol) == 0) return symbol; // found it
        currentNode=currentNode->next;
    } // while
    sprintf(errmsg,"Undeclared Symbol [%s]. ",symbol);
    error_message(errmsg);
    return NULL; // should never get here
}// symTabLookup


// add an ID to the symbol table
// abort if chk_dcl_flag and this is a duplicate name
char* addSymbolName(char* sname){
    symNode *newNode, *currentNode, *previousNode = NULL;

    // special logic for first time in here
    if (symListHead == NULL) {
        newNode = malloc(sizeof(symNode));
        newNode->name = strdup(sname);
        newNode->next = NULL;
        newNode->previous = NULL;
        symListHead= newNode;
        symListTail= newNode;
        symbolTable= newNode;
    } else {
        // not the firt time
        currentNode= symListHead;
        while (currentNode != NULL) {
            if (strcmp(currentNode->name,sname) == 0) {
                if (chk_decl_flag == 1) {
                    sprintf(errmsg,"Duplicate symbol name [%s]",sname);
                    error_message(errmsg);
                } else ; // duplicate symbol but no error checking, so now what?
            }
            previousNode=currentNode;
            currentNode=currentNode->next;
        }
        // adds symbol to the end of the table
        newNode = malloc(sizeof(symNode));
        previousNode->next = newNode;
        newNode->name = strdup(sname);
        newNode->next = NULL;
        newNode->previous=previousNode;
        symListTail=newNode;
        if ((strcmp(newNode->name,marker) == 0)) symListHead= newNode; //start of local variables
    }
    
    return sname;
}

// move the local symbols list
void moveLocalSymbols(char* fname){
    symNode *currentNode;
    if (symListHead==symbolTable) symbolTable=NULL; // this happens if there are no global symbols
    currentNode= symListHead;
    symListTail->next = symbolTable; //point the end of the local symbol table to the global symbol table
    symListTail=currentNode->previous; // end of global symbol table
    if (currentNode->previous != NULL) currentNode->previous->next = NULL;
    UpdateFuncListSymols(fname, currentNode->next);
    free(currentNode); // this is the marker node
    symListHead=symbolTable;
}

void freeMarker(void){  // special case in certain situations
    if (symbolTable != NULL)
        if (strcmp(symbolTable->name, marker) == 0) {
            free(symbolTable);
            symbolTable = NULL;
        }
}
