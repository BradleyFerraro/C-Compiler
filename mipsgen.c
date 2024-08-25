//
//  mipsgen.c
//  Compiler

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mipsgen.h"
#include "ast.h"
#include "helper.h"

//funcNode* funcList      =NULL;
extern symNode*  symbolTable ;
extern funcNode* funcList;
//extern char tmpStr[255];

extern symNode*  symListHead   ;
extern symNode*  symListTail   ;

//int tmpIdCnt=0, labelCnt=0;
//char tmpStr[255];
char p1[50], p2[50], p3[50], p4[50], p5[50];
char* GActiveFunctionName=NULL;
char* GCallingFunctionName=NULL;


void partStr(char* str, char* p1, char* p2, char* p3, char* p4, char* p5){
    int i=0, j=0;
    p1[0] = '\0'; p2[0] = '\0'; p3[0] = '\0'; p4[0] = '\0'; p5[0] = '\0';
    while (str[i] != '\0' && str[i] != ' ') {
        p1[j++] = str[i++];}
    p1[j]='\0';
    j=0; while(str[i] == ' ' && str[i] != '\0'){ i++;}
    while (str[i] != '\0' && str[i] != ' ') {
        p2[j++] = str[i++];}
    p2[j]='\0';
    j=0; while(str[i] == ' ' && str[i] != '\0'){ i++;}
    while (str[i] != '\0' && str[i] != ' ') {
        p3[j++] = str[i++];}
    p3[j]='\0';
    j=0; while(str[i] == ' ' && str[i] != '\0'){ i++;}
    while (str[i] != '\0' && str[i] != ' ') {
        p4[j++] = str[i++];}
    p4[j]='\0';
    j=0; while(str[i] == ' ' && str[i] != '\0'){ i++;}
    while (str[i] != '\0' && str[i] != ' ') {
        p5[j++] = str[i++];}
    p5[j]='\0';
}


void genPrintln(void){
    prt(".align 2 \n.data\n_nl: .asciiz \"\\n\"\n.align 2\n.text\n");
    prtln("main:");
    prtln("\tj _main");
    prt("_println:\n\tli $v0, 1\n");
    prt("\tlw $a0, 8($sp)\n\tsyscall\n\tli $v0, 4\n\tla $a0, _nl\n\tsyscall\n\tjr $ra\n");
    prt("\n");
    
}

void genGlobals(void){
    symNode *sNode = symbolTable;
    if (symbolTable!= NULL) prt(".align 2\n.data\n");
    while (sNode != NULL){
        // may want to make these names unique and add a field to the symbol table for the unique name
        prt(sNode->name);prt(":");prt("\t.word 0\n");
        sNode=sNode->next;
    } //while
    prt("\n");
}

/*void MGputLabel(char* label){
    printf("%s\n",label);
}
*/

/*
 void MGgetReturnValue(void){
    char tmpStr[255];
    sprintf(tmpStr,"\n#exiting function %s with return value",GActiveFunctionName);
    prtln(tmpStr);
    prtln("\tmove $sp, $fp  \t#restore the stack pointer");
    prtln("\tjr $ra \t#return\n");
}

char* MGexitFunction(ASTNode* node){
    char tmpStr[255];
    sprintf(tmpStr,"\n#exiting function %s",GActiveFunctionName);
    prtln(tmpStr);
    prtln("\tmove $sp, $fp  \t#restore the stack pointer");
    prtln("\tjr $ra \t#return\n");
    return NULL;
}
*/

char* MGid(ASTNode* node){
    return(node->name);
}

char* MGintconst(ASTNode* node){
    char tmpStr[255];
    sprintf(tmpStr,"%08x",atoi(node->name)); //8 characters with leading zeros
    //Load upper 16 bits of the constant
    prt("\tlui $t9, 0x");prtc(tmpStr[0]);prtc(tmpStr[1]);prtc(tmpStr[2]);prtc(tmpStr[3]);
    prtln("\t#Load upper 16 bits of the constant.");
    //load lower 16 bits of the constant (0x5678)
    prt("\tori $t9, $t9, 0x");prtc(tmpStr[4]);prtc(tmpStr[5]);prtc(tmpStr[6]);prtc(tmpStr[7]);
    prtln("\t#Load lower 16 bits of the constant.");
    return(node->name);
}

//x:=y
char* MGassg(ASTNode* node){
    char tmpStr[255];
    
    genMips(node->child0);
    genMips(node->child1); // generate mips for the children of this assignment statement
    comment(node->code);
    partStr(node->code, p1, p2, p3, p4, p5);
    
    //p1 := p3
    // put the value of p3 (whatever type it is) into register $t0
    if (getIdType(p3) == global){
        prt("\tlw $t0,");prtln(p3);
    } else if (getIdType(p3) == constant) {
        prtln("\tmove $t0, $t9 \t#constant");
    } else if (getIdType(p3) == functionresult) {
        prtln("\tmove $t0, $v0 \t#function result");
    } else { // Load value from the stack into a register $t0
        sprintf(tmpStr,"\tlw $t0, %d($fp)",getIdOffset(p3));
        prtln(tmpStr);
    }
    // make the assignment of $t0 to p1
    if (getIdType(p1) == global){
        prt("\tsw $t0,");prtln(p1);
    } else { // Load value from the stack into a register $t0
        // store $t0 on the stack location of p1
        sprintf(tmpStr,"\tsw $t0, %d($fp)",getIdOffset(p1));
        prtln(tmpStr);
    }
    return p1;
}


//x := -y
char* MGuminus(ASTNode* node){
    char tmpStr[255];
//    dest=newTemp();
//    sprintf(tmpStr,"%s := -%s",dest,genMips(node->child0));
 //   saveCode(tmpStr,node);
    comment(node->code);
    genMips(node->child0);
    partStr(node->code, p1, p2, p3, p4, p5);
    //p1 result temp var
    //p3 -source
    if (getIdType(&p3[1]) == global){
        prt("\tlw $t0,");prtln(&p3[1]);
    } else if (getIdType(&p3[1]) == constant) {
        prtln("\tmove $t0, $t9 \t#constant");
    } else if (getIdType(&p3[1]) == functionresult) {
        prtln("\tmove $t0, $v0 \t#function result");
    } else { // Load value from the stack into a register $t0
        sprintf(tmpStr,"\tlw $t0, %d($fp)",getIdOffset(&p3[1]));
        prtln(tmpStr);
    }
    // $t1 = 0 - $t0 now contains the source
    prtln("\tsub $t1,$zero,$t0 \t#unary minus");
    
    // make the assignment of $t1 to p1
    if (getIdType(p1) == global){
        prt("\tsw $t1,");prtln(p1);
    } else {
        // store $t0 on the stack location of p1
        sprintf(tmpStr,"\tsw $t1, %d($fp)",getIdOffset(p1));
        prtln(tmpStr);
    }


    return NULL;
}

//x := y op z op: ∈ {+, -, *, /}.
char* MGaop(ASTNode* node){
    char tmpStr[255];
    comment(node->code);
    genMips(node->child0); // generate mips for the children
    partStr(node->code, p1, p2, p3, p4, p5);
    //p1 := p3 aop p5
    // put p3 into a register
    if (getIdType(p3) == global){
        prt("\tlw $t0,");prtln(p3);
    } else if (getIdType(p3) == constant) {
        prtln("\tmove $t0, $t9 \t#constant");
    } else if (getIdType(p3) == functionresult) {
        prtln("\tmove $t0, $v0 \t#function result");
    } else { // Load value from the stack into a register
        sprintf(tmpStr,"\tlw $t0, %d($fp)",getIdOffset(p3));
        prtln(tmpStr);
    }
    // we need to store $t0 someplace because the next genMips could corrupt it.
    comment("store $t0 on the stack");
    prtln("\tsw $t0, -4($sp)  \t# store $t0 on the stack");
    prtln("\taddi $sp, $sp, -4 \t# decrement the stack pointer");

    genMips(node->child1);
 
    comment("restore $t0 from stack");
    prtln("\tlw $t0, 0($sp) \t# restore $t0 from stack");
    prtln("\taddi $sp, $sp, 4  \t# increment the stack pointer");

    
    partStr(node->code, p1, p2, p3, p4, p5);
    // put p5 into a register
    if (getIdType(p5) == global){
        prt("\tlw $t1,");prtln(p5);
    } else if (getIdType(p5) == constant) {
        prtln("\tmove $t1, $t9 \t#constant");
    } else if (getIdType(p5) == functionresult) {
        prtln("\tmove $t1, $v0 \t#function result");
    } else { // Load value from the stack into a register
        sprintf(tmpStr,"\tlw $t1, %d($fp)",getIdOffset(p5));
        prtln(tmpStr);
    }
    switch (p4[0]){
        case '+': prtln("\tadd $t3,$t0,$t1"); break;
        case '-': prtln("\tsub $t3,$t0,$t1"); break;
        case '*': prtln("\tmul $t3,$t0,$t1"); break;    //does not handle overflow
        case '/': prtln("\tdiv $t0, $t1"); prtln("\tmflo $t3"); break;   //ignores the remainder in hi
        default: printf("!error in MGaop, unknown arithmetic operator :%s",p4);
    }
    // make the assignment of $t3 to p1
    if (getIdType(p1) == global){
        prt("\tsw $t3,");prtln(p1);
    } else { // Load value from the stack into a register $t0
        // store $t3 on the stack location of p1
        sprintf(tmpStr,"\tsw $t3, %d($fp)",getIdOffset(p1));
        prtln(tmpStr);
    }

    return p1;
}


char* MGstmt_list(ASTNode* node){
    genMips(node->child0);
    while(node->child1 != NULL){
        node=node->child1;
        genMips(node->child0);
    }
    return strdup("");
}

/*arith_exp    :    ID
 |    INTCON
 |    arith_exp   arithop   arith_exp
 |    LPAREN   arith_exp   RPAREN
 |    opSUB   arith_exp
 |    fn_call
 */

// expr_list are the arguments to a function call
char* MGexpr_list(ASTNode* node){
    char str[255], tmpStr[255];
    ASTNode *next = node;
    sprintf(tmpStr," Arguments for this function call are: %s",node->code);
    comment(tmpStr); //list of arguments separated by spaces
    while (next != NULL) {
        // generate the code to populate this function call's arguments
        //if (next->child0->type != INTCONST) {
        genMips(next->child0);//}
        next = next->child1; // next parameter
    }
    // store parameters on the stack as indicated by the parameter list in node->code
    // but do it in reverse order
    int numParms=getParmCount(GCallingFunctionName);
    int offset=(numParms)*4;
    prtln("");
    comment("push parameters on the stack");
    int i=0;
    while (node->code[i] != '\0') {
        int j=0;
        while (node->code[i] != '\0' && node->code[i]  != ' ') {
            str[j++] = node->code[i++];
        }
        str[j]='\0';
        if (node->code[i]==' ') i++; //skip over a blank
        // str contains the actual parameter item which could be constant, global, functionresult, or is on the stack (temp, local)
        if (str[0] == '#') { // it's a constant
            sprintf(tmpStr,"%08x",atoi(&str[1])); //8 characters with leading zeros
            //Load upper 16 bits of the constant
            prt("\tlui $t0, 0x");prtc(tmpStr[0]);prtc(tmpStr[1]);prtc(tmpStr[2]);prtc(tmpStr[3]);
             prtln("\t#Load upper 16 bits of the constant.");
            //load lower 16 bits of the constant (0x5678)
            prt("\tori $t0, $t0, 0x");prtc(tmpStr[4]);prtc(tmpStr[5]);prtc(tmpStr[6]);prtc(tmpStr[7]);
             prtln("\t#Load lower 16 bits of the constant.");
        } else if (getIdType(str) == global){
            prt("\tlw $t0,");prt(str);prtln("\t #load global variable into register");
        } else if (getIdType(str) == functionresult){
            prt("\tmove $t0,$v0 \t #function result");
// *rcf* what happens if there are multiple function calls as parameters !!!
        } else { // Load value from the stack into a register $t0
            sprintf(tmpStr,"\tlw $t0, %d($fp)\t#load value from locals list into register",getIdOffset(str));
            prtln(tmpStr);
        }
        // push $t0 onto the stack
        sprintf(tmpStr,"\tsw $t0, -%d($sp) \t#put it on the stack",offset);
        prtln(tmpStr);
        offset=offset -4;
    }

    sprintf(tmpStr,"\taddi $sp, $sp, -%d \t#adjust the stack pointer",(numParms)*4);
    prtln(tmpStr);
   return "";
}


char* MGfunc_def(ASTNode* node){
    // prior to entry to this function the caller has pushed arguments, old $fp, and old $ra on the stack.
    // $fp = $sp and both are pointing at the old $ra.
    // We will create a linked list of nodes called the localsList that contains name, type and offset from $fp for the
    // parameters, local variables, and temps. We then access that list by name to get the type and offset of something.
    // The left most parameter will be offset +8 and subsequent parameters will increase the offset by 4.

    char tmpStr[255];
    comment(node->code);
    
    //node->code=begin function_name
    partStr(node->code,p1,p2,p3,p4,p5);
    GActiveFunctionName = strdup(p2); //store the current function name in a global variable for use later
    prtln(".align 2");prtln(".text");
    prt("_");
    prt(GActiveFunctionName);prtln(":");
    
    prtln("\tmove $fp $sp \t#start a new frame");

    //put location of parameters in locals list
    ASTNode *next= node->child1; //opt formals
    while (next !=NULL) {
        putParmsInLocalsList(next->name);
        next=next->child1;
    }

    // add local variables to the locals list if it's not a parameter.  Allocate space on the stack.
    symNode *symPtr=getLocalVariablesPtrForFunction(GActiveFunctionName); // get a pointer to the functions symboltable;
    while (symPtr != symbolTable && symPtr != NULL) {// not wraped around to the global symbol table
            if (! inLocalList(symPtr->name)) putVariableInLocalsList(symPtr->name,local);
            symPtr=symPtr->next;
    }
    

    // add temporaries to the locals list.  Allocate space on the stack.
    int numberOfTemporaries = atoi(p4) - atoi(p3);
    int i = 0;
    while (i < numberOfTemporaries) {
        sprintf(tmpStr, "_t%d", atoi(p3)+i);
        putVariableInLocalsList(tmpStr, temp);
        i ++;
    }
    
    genMips(node->child0);

    // generate function exit code
    sprintf(tmpStr,"\n#exiting function _%s",GActiveFunctionName);
    prtln(tmpStr);
    prtln("\tmove $sp, $fp  \t#restore the stack pointer");
    prtln("\tjr $ra \t#return\n");

    // clean up
    deallocateLocalsList();
    free(GActiveFunctionName);
    
   return strdup("");
}


char* MGfunc_call(ASTNode* node) {
    char tmpStr[255];
    prtln("");
    partStr(node->code,p1,p2,p3,p4,p5);
    //p1=call, p2=function name, p3 =num args
    GCallingFunctionName=strdup(p2);
    comment("set up for a function call");
    genMips(node->child0); // Generate code for the arguments and put them on the stack.
    partStr(node->code,p1,p2,p3,p4,p5); // added because genMips changes it
    // save the $fp and current $ra on the stack
    prtln("\taddi $sp, $sp, -4     # decrement the stack pointer");
    prtln("\tsw $fp, 0($sp)        # Store the value of $fp on of the stack");
    prtln("\taddi $sp, $sp, -4     # decrement the stack pointer");
    prtln("\tsw $ra, 0($sp)        # Store the value of $ra on of the stack");
    //prtln("\tmove $fp $sp          # set the new frame pointer.");

    prtln("");
    comment(node->code);

    prt("\tjal "); prt("_"); prt(p2); prt("\n");

    prtln("\tlw $ra, 0($sp)       #(restore return address");
    prtln("\tlw $fp, 4($sp)       #(restore frame pointer)");
    prtln("\tla $sp, 8($sp)       #(increment stack pointer)");
    
    sprintf(tmpStr,"\taddi $sp, $sp, %d \t#adjust stack pointer for arguments",4*atoi(p3));
    prtln(tmpStr);

    
    free(GCallingFunctionName);
    return "";
}


 
 char* MGrelop(ASTNode* node){
     char tmpStr[255];
     genMips(node->child0);  // generate mips for the children
     comment(node->code);
     char *label1 , *label2;
     label1 = newLabel();
     label2 = newLabel();
     partStr(node->code, p1, p2, p3, p4, p5);
     //p1 := p3 op p5
     // put p3 into a register
     if (getIdType(p3) == global){
         prt("\tlw $t0,");prtln(p3);
     } else if (getIdType(p3) == constant) {
         prtln("\tmove $t0, $t9 \t#constant");
     } else if (getIdType(p3) == functionresult){
         prt("\tmove $t0, $v0 \t#function result");
     } else { // Load value from the stack into a register
         sprintf(tmpStr,"\tlw $t0, %d($fp)",getIdOffset(p3));
         prtln(tmpStr);
     }
     
     // we need to store $t0 someplace because the next genMips could corrupt it.
     comment("store $t0 on the stack");
     prtln("\tsw $t0, -4($sp)  \t# store $t0 on the stack");
     prtln("\taddi $sp, $sp, -4 \t# decrement the stack pointer");

     genMips(node->child1);
  
     comment("restore $t0 from stack");
     prtln("\tlw $t0, 0($sp) \t# restore $t0 from stack");
     prtln("\taddi $sp, $sp, 4  \t# increment the stack pointer");

     partStr(node->code, p1, p2, p3, p4, p5);
     // put p5 into a register
     if (getIdType(p5) == global){
         prt("\tlw $t1,");prtln(p5);
     } else if (getIdType(p5) == constant) {
         prtln("\tmove $t1, $t9 \t#constant");
     } else if (getIdType(p5) == functionresult){
         prtln("\tmove $t1, $v0 \t#function result");
     } else { // Load value from the stack into a register
         sprintf(tmpStr,"\tlw $t1, %d($fp)",getIdOffset(p5));
         prtln(tmpStr);
     }
     switch (node->type){
         case EQ: sprintf(tmpStr, "\tbeq $t0,$t1,_%s", &label1[1]); break;
         case NE: sprintf(tmpStr, "\tbne $t0,$t1,_%s", &label1[1]); break;
         case GT: sprintf(tmpStr, "\tbgt $t0,$t1,_%s", &label1[1]); break;
         case GE: sprintf(tmpStr, "\tbge $t0,$t1,_%s", &label1[1]); break;
         case LT: sprintf(tmpStr, "\tblt $t0,$t1,_%s", &label1[1]); break;
         case LE: sprintf(tmpStr, "\tble $t0,$t1,_%s", &label1[1]); break;
         default: printf("!error in MGrelop unknown logical operator :%s",p4);
     }
     prtln(tmpStr);
     
     // load 0 into $t3 false
     prtln("\tli $t3 0 \t#false");
     sprintf(tmpStr, "\tj _%s \t#skip over next instruction", &label2[1]);
     prtln(tmpStr);
     // load 1 into $t3 true
     prt("_");
     prt(&label1[1]);
     prtln(":");
     prtln("\tli $t3 1 \t#true");
     prt("_");
     prt(&label2[1]);
     prtln(":");
     // store $t3 on the stack location of p1 because p1 must be a temporary
     sprintf(tmpStr,"\tsw $t3, %d($fp)",getIdOffset(p1));
     prtln(tmpStr);

     return p1;
 }

char* MGlogop(ASTNode* node){
    char tmpStr[255];
    char* label1 = newLabel();
    
    genMips(node->child0); // generate mips for the children
    comment(node->code);
    partStr(node->code, p1, p2, p3, p4, p5);
    //t := v1 op v2
    // put p3 into a register
    if (getIdType(p3) == global){
        prt("\tlw $t0,");prtln(p3);
    } else if (getIdType(p3) == constant) {
        prtln("\tmove $t0, $t9 \t#constant");
    } else if (getIdType(p3) == functionresult) {
        prtln("\tmove $t0, $v0 \t#function result");
    } else { // Load value from the stack into a register
        sprintf(tmpStr,"\tlw $t0, %d($fp)",getIdOffset(p3));
        prtln(tmpStr);
    }

    //short circuit evaluation
    // if the first operand of the && is 0, store the result in $t3,
    // and jump past the second check
    if (node->type == AND) {
        prtln("\tadd $t3, $zero, $t0");
        sprintf(tmpStr, "\tbeq $t0, $zero, _%s", &label1[1]);
        prtln(tmpStr);
    }
    
    // short circuit evaluation
    // if the first operand of the || is 1, store the result in $t3,
    // and jump past the second check
    if (node->type == OR) {
        prtln("\tadd $t3, $zero, $t0");
        sprintf(tmpStr, "\tbne $t0, $zero, _%s", &label1[1]);
        prtln(tmpStr);
    }
    
    // we need to store $t0 someplace because the next genMips could corrupt it.
    comment("store $t0 on the stack");
    prtln("\tsw $t0, -4($sp)  \t# store $t0 on the stack");
    prtln("\taddi $sp, $sp, -4 \t# decrement the stack pointer");

    genMips(node->child1);

    comment("restore $t0 from stack");
    prtln("\tlw $t0, 0($sp) \t# restore $t0 from stack");
    prtln("\taddi $sp, $sp, 4  \t# increment the stack pointer");

    partStr(node->code, p1, p2, p3, p4, p5);

    // put p5 into a register
    if (getIdType(p5) == global){
        prt("\tlw $t1,");prtln(p5);
    } else if (getIdType(p5) == constant) {
        prtln("\tmove $t1, $t9 \t#constant");
    } else if (getIdType(p5) == functionresult){
        prtln("\tmove $t1, $v0 \t#function result");
    } else { // Load value from the stack into a register
        sprintf(tmpStr,"\tlw $t1, %d($fp)",getIdOffset(p5));
        prtln(tmpStr);
    }
    
    // switch
    switch (node->type){
        case AND: prtln("\tand $t3, $t0, $t1"); break;
        case OR: prtln("\tor $t3, $t0, $t1"); break;
        default: printf("!error in MGlogop, unknown logical operator :%s",p4);
    }
    
    sprintf(tmpStr, "_%s:", &label1[1]);
    prtln(tmpStr);

    // make the assignment of $t3 to p1
    if (getIdType(p1) == global){
        prt("\tsw $t3,");prtln(p1);
    } else { // Load value from the stack into a register $t0
        // store $t3 on the stack location of p1
        sprintf(tmpStr,"\tsw $t3, %d($fp)",getIdOffset(p1));
        prtln(tmpStr);
    }

    return p1;
}


char* MGif(ASTNode* node){
    char tmpStr[255];
    //if p2 l1 l2
    genMips(node->child0); //boolean expression
    comment(node->code);
    partStr(node->code,p1,p2,p3,p4,p5);
    sprintf(tmpStr,"\tlw $t1, %d($fp) \t#get the result of the conditon",getIdOffset(p2));
    prtln(tmpStr);
    sprintf(tmpStr, "\tbeq $t1,$zero,_%s \t# false, skip to else, first label in 3 addr code", &p3[1]);
    prtln(tmpStr);
    // then case here
    genMips(node->child1);
    partStr(node->code,p1,p2,p3,p4,p5);
    sprintf(tmpStr,"\t j _%s \t# end of then case, jump second label in 3 addr code", &p4[1]);
    prtln(tmpStr);
    // else case here
    sprintf(tmpStr,"_%s:", &p3[1]);
    prtln(tmpStr);
    genMips(node->child2); // else case
    partStr(node->code,p1,p2,p3,p4,p5);
    sprintf(tmpStr,"_%s:", &p4[1]);
    prtln(tmpStr);
   return NULL;
}

char* MGwhile(ASTNode* node){
    char tmpStr[255];
    comment(node->code);
    //child0 is the condition
    //child1 is the code
    partStr(node->code,p1,p2,p3,p4,p5);
    // p1 is label1
    // p3 is the temporary that contains the condition result
    // p5 is the label to jump to if the condition is false
    
    // label:
    prt("_");
    prt(&p1[1]);prtln(":");
    genMips(node->child0);
    partStr(node->code,p1,p2,p3,p4,p5);
    // test the condiiton and branch if false
    sprintf(tmpStr,"\tlw $t1, %d($fp) \t#get the result of the conditon",getIdOffset(p3));
    prtln(tmpStr);
    sprintf(tmpStr, "\tbeq $t1,$zero,_%s \t# false, skip to the end label", &p5[1]);
    prtln(tmpStr);
    genMips(node->child1);
    partStr(node->code,p1,p2,p3,p4,p5);
    // jump back up the while loop
    sprintf(tmpStr,"\t j _%s \t# jump back to while", &p1[1]);
    prtln(tmpStr);
    // exit loop label
    partStr(node->code,p1,p2,p3,p4,p5);
    prt("_");
    prt(&p5[1]);prtln(":");

    return NULL;
}

char* MGreturn(ASTNode* node){
    char tmpStr[255];
    comment(node->code);
    if (node->child0 != NULL){
        genMips(node->child0);
        // return x
        partStr(node->code,p1,p2,p3,p4,p5);
        // load p2 into $v0
        if (getIdType(p2) == global){
            prt("\tlw $v0,");prtln(p2);
        } else if (getIdType(p2) == constant) {
            prtln("\tmove $v0, $t9 \t#constant");
        } else if (getIdType(p2) == functionresult) {
            prtln("\tmove $v0, $v0 \t#function result do nothing really");
        } else { // Load value from the stack into a register
            sprintf(tmpStr,"\tlw $v0, %d($fp)",getIdOffset(p2));
            prtln(tmpStr);
        }
    }
    sprintf(tmpStr,"\n#exiting function _%s",GActiveFunctionName);
    prtln(tmpStr);
    prtln("\tmove $sp, $fp  \t#restore the stack pointer");
    prtln("\tjr $ra \t#return\n");

    return NULL;
}




char* genMips(ASTNode* node){
    if (node == NULL) return "";
    //indent(rCount); printf("%s %s\n",getTypeName(node->type),node->name);
    switch (node->type) {
        case DUMMY:     return NULL;
        case FUNC_DEF:  return MGfunc_def(node);
        case FUNC_CALL: return MGfunc_call(node);
        case IF:        return MGif(node);
        case WHILE:     return MGwhile(node);
        case ASSG:      return MGassg(node);
        case RETURN:    return MGreturn(node);
        case STMT_LIST: return MGstmt_list(node);
        case EXPR_LIST: return MGexpr_list(node);
        case IDENTIFIER:return MGid(node);
        case INTCONST:  return MGintconst(node);
        // relational ops
        case EQ:
        case NE:
        case LE:
        case LT:
        case GE:
        case GT:        return MGrelop(node);
       //arihmetic ops
        case ADD:
        case SUB:
        case MUL:
        case DIV:       return MGaop(node);
        case UMINUS:    return MGuminus(node);
        //logical ops
        case AND:
        case OR:        return MGlogop(node);
        default:
            break;
    }
    return("");
}

