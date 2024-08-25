// Bradley Ferraro 
// Assignment 1 Milestone 1
// CS 453

// scanner.c file containing a get_tokens func to act as a 
// premliminary scanner for assignment 1

// credit to chat GPT
// I originally asked chat gpt how to lay out the get token function
// and it gave me this line: "while ((ch = getchar()) != EOF) {}"
// which helped me understand how to take from standard input
// continuously. I also googled how to put a char back on to the
// stdin stream and I found the "ungetc()" funtion on stackexchange.
// The rest was utilizing a case statement and tweaking each case
// to work for single, double, integer constant, and lexeme blocks.

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "scanner.h"
#include "helper.h"

char* lexeme = NULL;
int line_num = 1;


// This funtion checks to see if the string is a keyword. if it is
// return the token for it, or ID meaning it is an identifier
int is_keyword(char* str) {
	if (strcmp(str, "int")      == 0) return kwINT;
	if (strcmp(str, "if")       == 0) return kwIF;
	if (strcmp(str, "else")     == 0) return kwELSE;
	if (strcmp(str, "while")    == 0) return kwWHILE;
	if (strcmp(str, "return")   == 0) return kwRETURN;
	return ID; // not a key word must be an identifier
}

// This was originally inside the get_token function but it 
// looked nicer out here. It parses through a comment until
// it finds the end and returns the next char. 
// What about a comment that never ends?
// That wasn't in the spec so I think we will implement
// logic checking for comments later.
char skipBlockComment(char ch) {
    if (ch == '/') {
        char next_ch = getchar();
        if (next_ch == '*') { 
            char last_ch = 0;
            while ((ch = getchar()) != EOF) {
                if (last_ch == '*' && ch == '/') {
                    return getchar();
                }
                last_ch = ch;
            }
        } else {
            ungetc(next_ch, stdin); 
            return ch;
        }
    }
    return ch;
}

// Gets the next token from the input stream.
// this is called by get_token defined below that can add debug prints
Token getToken(void) {
	static char buffer[256];
    char emsg[255];
	char ch;
	char next_ch;
	int i = 0;

    while ((ch = getchar()) != EOF) { // read the next character fromk the input stream
        ch = skipBlockComment(ch); // check for comment and skip over it
        if (isspace(ch)) {
          // found white space character
          if (ch == '\n') line_num++; // char is a new line
            continue; // skip over white space
		}
 		switch (ch) { // we are here because ch is not newline,comment or whitespace
			// single character tokens
			case '(': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer;
				return LPAREN;
			case ')': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer;
				return RPAREN;
			case '{': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer;
				return LBRACE;
			case '}': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer;
				return RBRACE;
			case ',': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer;
				return COMMA;
			case ';': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer;
				return SEMI;
			case '+': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer;
				return opADD;
			case '-': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer; 
				return opSUB;
			case '*': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer; 
				return opMUL;
			case '/': 
				buffer[i++] = ch; 
				buffer[i++] = '\0';
				lexeme = buffer; 
				return opDIV;


			// single or double char tokens
			case '=':
			next_ch = getchar();
			if (next_ch != '=') {
				ungetc(next_ch, stdin);
				buffer[i++] = ch; 
				buffer[i++] = '\0'; 
				lexeme = buffer;
				return opASSG;
			} else { // ==
				buffer[i++] = ch;
                buffer[i++] = ch;
				buffer[i++] = '\0'; 
				lexeme = buffer;
				return opEQ;
			}
			case '>':
			next_ch = getchar();
			if (next_ch != '=') {
				ungetc(next_ch, stdin);
				buffer[i++] = ch; 
				buffer[i++] = '\0'; 
				lexeme = buffer;
				return opGT;
			} else { // >=
				buffer[i++] = ch;
				buffer[i++] = next_ch;
				buffer[i++] = '\0'; 
				lexeme = buffer;
				return opGE;
			}
			case '<':
			next_ch = getchar();
			if (next_ch != '=') {
				ungetc(next_ch, stdin);
				buffer[i++] = ch; 
				buffer[i++] = '\0'; 
				lexeme = buffer;
				return opLT;
			} else { // <=
				buffer[i++] = ch;
				buffer[i++] = next_ch;
				buffer[i++] = '\0'; 
				lexeme = buffer;
				return opLE;
			}
			case '!':
			next_ch = getchar();
			if (next_ch != '=') {
				ungetc(next_ch, stdin);
                buffer[i++] = ch;
                buffer[i++] = '\0';
                lexeme = buffer;
                return opNOT;
			} else {
				buffer[i++] = ch;
				buffer[i++] = next_ch;
				buffer[i++] = '\0'; 
				lexeme = buffer;
				return opNE;
			}
			case '&':
			next_ch = getchar();
			if (next_ch != '&') {
				ungetc(next_ch, stdin);
                // rcf an ampersand that is not followed by another one is a syntax error
                error_message("Syntax error single & found.");
			} else {
				buffer[i++] = ch;
				buffer[i++] = next_ch;
				buffer[i++] = '\0'; 
				lexeme = buffer;
				return opAND;
			}
			case '|':
			next_ch = getchar();
			if (next_ch != '|') {
				ungetc(next_ch, stdin);
                // rcf a pipe that is not followed by another one is a syntax error
                error_message("Syntax error single | found.");
			} else {
				buffer[i++] = ch;
				buffer[i++] = next_ch;
				buffer[i++] = '\0'; 
				lexeme = buffer;
				return opOR;
			}

			// if its not one of the above tokens, it must be an INTCON or an ID
			default:
				if (isdigit(ch)) {
					buffer[i++] = ch;
					while (isdigit(ch = getchar())) {
						buffer[i++] = ch;
					}
                    // rcf ch must be a valid terminator for a number !!!
                    // not sure what all those thing are, like ! & ( ???
                    // for now just check that it is not an alpha
                    if (isalpha(ch)) {  // an INTCON can not be digits terminated by a letter
                        error_message("Invalid terminator for an INTCON");
                    }
					ungetc(ch, stdin); // unget the unneeded character
					buffer[i] = '\0';
					lexeme = buffer;
					return INTCON;
				} else if (isalpha(ch)) {
					buffer[i++] = ch;
                    ch = getchar();
					while (isalpha(ch)||isdigit(ch) ||ch == '_') { //isalnum(ch) does not work on lectura
						buffer[i++] = ch;
                        ch = getchar();
					}
					ungetc(ch, stdin); // unget the unneeded character
					buffer[i] = '\0';
					lexeme = buffer;
					return is_keyword(buffer);
                } else { // something is wrong
                    sprintf(emsg,"Syntax error in get_token ch=%c",ch);
                    error_message(emsg);
                }
		} //switch
	} //while not EOF
	return EOF; 
} //get_token


// add debug prints to getToken()
Token get_token(void){
    Token t = getToken();
    if (dbg) {
        if (t != EOF) printf("%s(%s) \n",getTokenName(t),lexeme);
        if (t == SEMI || t== EOF) printf("\n");
    }
  return(t);
}
