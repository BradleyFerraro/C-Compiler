//
//  codegen.c
//  Compiler
//
#include <stdio.h>
#include <string.h>
#include "codegen.h"
#include "ast.h"
#include "helper.h"

int tmpIdCnt=0, labelCnt=0;

void printIt(char *text){
   // single point for all print statements so they can be easily disabled
   //printf("%s",text);
}

void saveCode(char* code, ASTNode* node){
    char tmpStr[255];
    node->code=strdup(code);
    sprintf(tmpStr," %s\n",node->code);
    printIt(tmpStr);
}
void putLabel(char* label){
    char tmpStr[255];
    sprintf(tmpStr,"%s\n",label);
    printIt(tmpStr);
}

void getReturnValue(void){
    printIt("!get return value\n");
}

char* newTemp(void){
    char str[255];
    sprintf(str,"_t%i",tmpIdCnt++);
    return strdup(str);
}


char* exitFunction(ASTNode* node){
    char tmpStr[255];
    //node->child1 = opt_formals;
    //node->args
    char str[255];
    sprintf(str,"!end function %s",node->name);
    sprintf(tmpStr,"%s\n\n",str);
    printIt(tmpStr);
    return strdup(str);
}


char* CGid(ASTNode* node){
    return(strdup(node->name));
}
char* CGintconst(ASTNode* node){
    char tmpStr[255];
    sprintf(tmpStr,"#%s",node->name);
    return(strdup(tmpStr));
}


//x := y
char* CGassg(ASTNode* node){
    char tmpStr[255];
    char *dest;
    dest=genCode(node->child0);
    sprintf(tmpStr,"%s := %s",dest,genCode(node->child1));
    saveCode(tmpStr,node);
    return strdup(dest);
}


//x := -y
char* CGuminus(ASTNode* node){
    char tmpStr[255];
    char *dest;
    dest=newTemp();
    sprintf(tmpStr,"%s := -%s",dest,genCode(node->child0));
    saveCode(tmpStr,node);
    return strdup(dest);
}

//x := y op z op: ∈ {+, -, *, /}.
char* CGaop(ASTNode* node){
    char tmpStr[255];
    char *dest;
    dest=newTemp();
    sprintf(tmpStr,"%s := %s %s %s",dest,genCode(node->child0),node->name,genCode(node->child1));
    saveCode(tmpStr,node);
    return strdup(dest);
}

char* CGstmt_list(ASTNode* node){
    genCode(node->child0);
    while(node->child1 != NULL){
        node=node->child1;
        genCode(node->child0);
    }
    return strdup("");
}

char* CGexpr_list(ASTNode* node){
    char tmpStr[1024]; // note that this may not be long enough for a parameter list of unknown length
    tmpStr[0]='\0';
    ASTNode *next = node;
    while (next != NULL) {
        strcat(tmpStr,genCode(next->child0));
        strcat(tmpStr," ");
        next = next->child1;
    }
    tmpStr[strlen(tmpStr)]='\0'; // remove trailing space
    saveCode(tmpStr,node);
    return "";
}

char* CGfunc_def(ASTNode* node){
    int tmpcountlow, tmpcounthigh;
    tmpcountlow = tmpIdCnt;
    char tmpStr[512];
    ASTNode *next= node->child1; //opt formals
    while (next !=NULL) {
        next->code=strdup(next->name);
        next=next->child1;
    }
    genCode(node->child0);
    tmpcounthigh = tmpIdCnt;
    sprintf(tmpStr,"!begin %s %d %d",node->name, tmpcountlow, tmpcounthigh);
    saveCode(tmpStr,node);
    return strdup("");
}

char* CGfunc_call(ASTNode* node){
    char tmpStr[255];
    genCode(node->child0);
    sprintf(tmpStr,"call %s %i",node->name,node->args);
    saveCode(tmpStr,node);
    return "%FunctionResult";
}

char* CGlogop(ASTNode* node){
    char tmpStr[255];
    char* tmpVar=newTemp();
    sprintf(tmpStr,"%s := %s %s %s",tmpVar,genCode(node->child0),node->name,genCode(node->child1));
    saveCode(tmpStr,node);
    return tmpVar;
}

char* CGrelop(ASTNode* node){
    char tmpStr[255];
    char* tmpVar=newTemp();
    sprintf(tmpStr,"%s := %s %s %s",tmpVar,genCode(node->child0),node->name,genCode(node->child1));
    saveCode(tmpStr,node);
    return tmpVar;
}

char* CGif(ASTNode* node){
    char tmpStr[255];
    char *label1 , *label2, *conVar;
    label1=newLabel();
    label2=newLabel();
    conVar=genCode(node->child0); //boolean expression
    sprintf(tmpStr,"if %s %s %s",conVar,label1, label2);
    saveCode(tmpStr,node);
    genCode(node->child2); // else case
    sprintf(tmpStr,"goto %s\n%s",label2,label1);
    printIt(tmpStr);
    genCode(node->child1); //then case
    putLabel(label2);
   return NULL;
}

char* CGwhile(ASTNode* node){
    char tmpStr[255];
    char *label1 , *label2, *condVar;
    label1=newLabel();
    label2=newLabel();
    condVar=genCode(node->child0);
    sprintf(tmpStr,"%s while %s end %s",label1,condVar,label2);
    saveCode(tmpStr,node);
    genCode(node->child1);
    putLabel(label2);
   return NULL;
}

char* CGreturn(ASTNode* node){
    char tmpStr[255];
    if (node->child0 == NULL)
        saveCode(node->name,node);
    else{
        sprintf(tmpStr,"return %s",genCode(node->child0));
        saveCode(tmpStr,node);
    }
    return NULL;
}



char* genCode(ASTNode* node){
    if (node == NULL) return "";
    //indent(rCount); printf("%s %s\n",getTypeName(node->type),node->name);
    switch (node->type) {
        case DUMMY:     return NULL;
        case FUNC_DEF:  return CGfunc_def(node);
        case FUNC_CALL: return CGfunc_call(node);
        case IF:        return CGif(node);
        case WHILE:     return CGwhile(node);
        case ASSG:      return CGassg(node);
        case RETURN:    return CGreturn(node);
        case STMT_LIST: return CGstmt_list(node);
        case EXPR_LIST: return CGexpr_list(node);
        case IDENTIFIER:return CGid(node);
        case INTCONST:  return CGintconst(node);
        // relational ops
        case EQ:
        case NE:
        case LE:
        case LT:
        case GE:
        case GT:        return CGrelop(node);
       //arihmetic ops
        case ADD:
        case SUB:
        case MUL:
        case DIV:       return CGaop(node);
        case UMINUS:    return CGuminus(node);
        //logical ops
        case AND:
        case OR:        return CGlogop(node);
        default:
            break;
    }
    return("");
}
