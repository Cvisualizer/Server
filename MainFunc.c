#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "SymTable.h"
#include "VSM.h"

int StartP = 0, SymPrintSW = 0;
int returnVal;
static int ExecSW = 1, ObjOutSW = 0, TraceSW = 0, StatSW = 0;
static int ErrorC = 0;
static char SourceFile[20];
extern FILE *yyin;
static void SetUpOpt(int,char *[]);

int main(int argc, char *argv[]){

  SetUpOpt(argc, argv);
  if(SourceFile[0] != '\0'){
    if((yyin = fopen(SourceFile, "r")) == NULL){
      fprintf(stderr, "Source file cannot be opened.");
      exit(-1);
    }
  }
  yyparse();
  // dump all Instruction Codes.
  // DumpIseg(0, PC() - 1);
  
  // final data of Symbol Table.
  //printf("\n -*- Symbol Table for Defined Functions -*- \n\n");
  PrintAllSymEntry();
  
  // the symbol table for local variables.
  //printf("\n -*- Symbol Table for Local Vars and Parameters (USTbl[]) -*- \n\n");
  dumpUSTbl();
  //freeUSTbl();
  
  printf(" \"code\" : \n");
  // the structure for function and frame size
  //printf("\n -*- Structure for function name and the number of local variables -*- \n\n");
  dumpFuncFrameStr();
  //if(ExecSW || TraceSW)
    if(1){
      if(SourceFile[0] == '\0')
        rewind(stdin);
      returnVal = StartVSM(StartP,TraceSW);
      //printf("\t\t\t}\n");
      if(returnVal != 0){
        //printf("(main) Execution aborted !\n");
        //printf("The return value is [%d]\n",returnVal);
      }
      if(StatSW) {
	ExecReport();
      }
      // ERROR
    /* }else{ */
    /*   printf("Execution suppressed\n"); */
    /* } */
    }
}
static void SetUpOpt(int argc, char *argv[])
{
  char *s;
  if(--argc > 0 && (*++argv)[0] == '-'){
    for(s = *argv+1; *s != '\0' ; s++)
      switch(tolower(*s)){
        case 'c' : StatSW   = 1;  break;
        case 'd' : DebugSW  = 1;  break;
        case 'n' : ExecSW   = 1;  break;
        case 'o' : ObjOutSW = 1;  break;
        case 's' : SymPrintSW = 1; break;
        case 't' : TraceSW = 1;    break;
      }
    argc--;
    argv++;
  }
  if(argc > 0)
    strcpy(SourceFile, *argv);
}

void yyerror(char *msg)
{
  extern int yylineno;
  printf("%s at line %d\n", msg, yylineno);
  ErrorC++;
}
