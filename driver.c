/*
 * File: parser-driver.c
 * Author: Saumya Debray
 * Purpose: The driver code for the CSC 453 project.  It uses command-line
 *          arguments to control whether semantic checks and/or code 
 *          generation is carried out.
 */




#include <stdio.h>
#include <string.h>

extern int parse(void);  // defined in parser.c

int chk_decl_flag = 0;      /* set to 1 to do semantic checking */
int print_ast_flag = 0;     /* set to 1 to print out the AST */
int gen_code_flag = 0;      /* set to 1 to generate code */
int dbg=0;                  /* print debug info */
/*
 * parse_args() -- parse command-line arguments and set flags appropriately
 * to control the actions performed by the compiler.  The arguments are:
 *
 *    --chk_decl     : to check legality of declarations
 *    --print_ast    : to print out the AST of each function
 *    --gen_code     : to generate code
 *    --dbg          : print debug info
 */
void parse_args(int argc, char *argv[]) {
  int i;
  for (i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "--chk_decl") == 0)       {chk_decl_flag = 1;}
      else if (strcmp(argv[i], "--print_ast") == 0) {print_ast_flag = 1;}
      else if (strcmp(argv[i], "--gen_code") == 0)  {gen_code_flag = 1;}
      else if (strcmp(argv[i], "--dbg") == 0)       {dbg = 1;}
      else { fprintf(stderr, "Unrecognized option: %s\n", argv[i]); }
    } //if
  } //for
} //parse_args

int main(int argc, char *argv[]) {
  int error_code;

  // chk_decl_flag = 1;

  // freopen( "/Users/rferraro/ccode/BradleyHomework/compilers/compiler/compiler/test", "r", stdin );
  // gen_code_flag = 1;

  //print_ast_flag = 1;
  //dbg=1;
 

  parse_args(argc, argv);
  error_code = parse();
  return error_code;
}

