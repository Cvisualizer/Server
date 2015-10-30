#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SymTable.h"

static int Level = 0, Mallocp = 0, MaxFSZ;
static char msg[50];

#define SYMT_SIZE 200
static STentry SymTab[SYMT_SIZE];
static STP SymTptr = &SymTab[1];

#define BT_SIZE 20
static struct {
  int allocp;
  STP first;
} BLKtbl[BT_SIZE] = {{0, &SymTab[1]}};

extern int SymPrintSW;
/*
 * global variables for local Vars and Parameters.
*/
static char *function;
static int functionParseState = OUT;

/*
  to make users allow C language syntax.
 */
static int afterFuncDefinition = 0;

#define UNITE_ST_SIZE 35
static int USTCounter = 0;
static int USTPointer = 0;
static struct {
  char *fname;
  char *name;
  int type;
  Class c;
  int rltvAdd;
} USTbl[UNITE_ST_SIZE];
/*
 * structure for controlling
 * both function names and MaxFSZ
*/
#define FuncMaxFSZStructureSize 20
static int FFpointer = 0;
static struct {
  // the function name.
  char *fname;
  // local var + next index of Iseg Instruction Code.
  int framesize;
} FuncFrame[FuncMaxFSZStructureSize];

/* structure for controlling
   local and global variable and function scope
   by censoring indexes of Instruction codes. */

/* the size of array
   for censoring instruction codes. */

#define IC_SIZE 40
static int ICPointer = 0;
static struct {
  char *fname;
  char *varname;
  int start;
  int end;
} IC_Censor[IC_SIZE];

void registerIsegIndex(int startIndex)
{
  STP p;
  // do nothing. this is useless function.
}

int getFuncParam(int location)
{
  
  STP p;
  char *function;
  int i, val1, val2, count = 0;
  // SymTab : table for variables and functions
  // defined on top level.
  for(p = &SymTab[1]; p < SymTptr; p++){
    if(location == p->loc){
      // Check type : Is this record of the Symbol Table "function" ?
      // Remove the posibility that a location of "variable number" is same as that of "function".
      if(p->class == FUNC){
	printf("\t\t{ \"call\"   :  { \"function\"  : \"%s\" , ", p->name);
	function = p->name;
      }
    }
  }
  // United Symbol Tabls :
  // table for params and local variables defined inside functions.
  // record : fname, name, type, c, rltvAdd
  for(i = 0; i < USTPointer; i++){
    // the function name of called equals that of symbol table ?
    val1 = strcmp(function, USTbl[i].fname);
    // and the class of variable number is "PARAM" ?
    if(USTbl[i].c == PARAM){
      val2 = 1;
    }else{
      val2 = 0;
    }
    if(val1 == 0){
      if(val2 == 1){
	count = count + 1;
      }
    }
  }
  return count;
}

void *xmalloc(int size)
{
  void *p;
  p = (void *) malloc(sizeof(size));
  if(p == NULL){
    printf("failed to malloc. exit this program.\n");
    exit(-2);
  }
  return p;
}

void saveVarInfo(char *Name, int Type, Class C)
{
  // -*- DEBUG -*-
  if(USTPointer >= UNITE_ST_SIZE){
    printf("Stack for local variables over flow .\nToo many local and global variables.");
    exit(-2);
  }
  
  USTbl[USTPointer].fname = xmalloc(sizeof(function) + 1);
  strcpy(USTbl[USTPointer].fname, function);
  USTbl[USTPointer].name = xmalloc(sizeof(Name) + 1);
  strcpy(USTbl[USTPointer].name, Name);
  USTbl[USTPointer].type = Type;
  USTbl[USTPointer].c = C;
  USTbl[USTPointer++].rltvAdd = Mallocp - 1;
  
}

void dumpUSTbl(void)
{
  static char *SymC[] = { "newsym", "func", "param", "var", "p_type" };
  static char *SymD[] = { "void", "int" } ;
  int i;
  
  printf(" \"local\" : [ \n");
  if(USTPointer == 0){
    printf("\t\t] ,\n");
    return;
  }
  // JSON  
  for(i = 0; i < USTPointer; i++){
    printf("\t\t { \"function\" : \"%s\", \"var\" : \"%s\", \"type\" : \"%s\", \"class\" : \"%s\" } ",
           USTbl[i].fname, USTbl[i].name, SymD[USTbl[i].type], SymC[USTbl[i].c]);
      // final loop ?
    if(i == (USTPointer - 1)){
      printf("\t],\n");
    }else{
      printf(", \n");
    }
  }
}

void freeUSTbl(void)
{
  int i;
  for(i = 0; i < USTPointer; i++){
    if(USTbl[i].fname != NULL){
      free(USTbl[i].fname);
    }
    if(USTbl[i].name != NULL){
      free(USTbl[i].name);
    }
  }
}

void OpenBlock(void)
{
  
  BLKtbl[++Level].first = SymTptr;
  BLKtbl[Level].allocp = Mallocp;
  
}

void CloseBlock(void)
{
  
  if(Mallocp > MaxFSZ){
    MaxFSZ = Mallocp;
  }
  SymTptr = BLKtbl[Level].first;
  Mallocp = BLKtbl[Level--].allocp;
  
}

static STP MakeEntry(char *Name, Dtype T, Class C)
{
  
  char *c[] = { "NEWSYM", "FUNC", "PARAM", "VAR", "F_PROT" };
  char *t[] = { "VOID" , "INT" };
  
  STP p;
  if((p = SymTptr++) >= &SymTab[SYMT_SIZE]){
    fprintf(stderr,"Too many symbols declared");
    exit(-2);
  }
  
  p->type   = T;
  p->class = C;
  p->deflvl = Level;
  p->name = Name;
  
  return p;
}

static STP LookUp(char *Name)
{
  STP p;
  for(p = SymTptr-1, SymTab[0].name = Name; p->name != Name; p--);
  return p > SymTab ? p : NULL;
}

STP MakeFuncEntry(char *Fname)
{
  
  STP p;
  p = LookUp(Fname);
  if(p == NULL)
    p = MakeEntry(Fname, VOID, NEWSYM);
  else if(p->class != F_PROT)
    yyerror("The Function name already declared");
  
  PrintSymEntry(p);
  Mallocp = MaxFSZ = 1;
  OpenBlock();
  return p;
}

void Prototype(STP FuncP,Dtype T){
  
  FuncP->class = F_PROT;
  FuncP->type = T;
  FuncP->loc = -1;
  FuncP->Nparam = SymTptr - BLKtbl[Level].first;
  
  if(Level > 1)
    yyerror("The  prototype declaration ignored");
  CloseBlock();
}

void FuncDef(STP Funcp, Dtype T){
  
  int n = SymTptr - BLKtbl[Level].first;
  if(Funcp->class == NEWSYM){
    Funcp->type = T;
    Funcp->Nparam = n;
  }else if(Funcp->class == F_PROT){
    if(Funcp->type != T)
      yyerror("Functoin type unmatched to the prototype");
    if(Funcp->Nparam != n)
      yyerror("No. of parameters mismatched");
    Bpatch(Funcp->loc, PC() + 3);
  }else{
    yyerror("The funciton already declared");
    return;
  }
  Funcp->class = FUNC;
  Funcp->loc = PC() + 3;
}

void EndFdecl(STP Funcp)
{
  
  CloseBlock();
  if(Funcp->class == FUNC){
    Bpatch(Funcp->loc, MaxFSZ);
  }
  
  /* processing related to function names and function frame sizes */
  FuncFrame[FFpointer].fname = xmalloc(strlen(Funcp->name) + 1);
  strcpy(FuncFrame[FFpointer].fname, Funcp->name);
  FuncFrame[FFpointer++].framesize = MaxFSZ;
  PrintSymEntry(Funcp);
  afterFuncDefinition = 1;
  
}

void VarDecl(STP Funcp, char *Name,Dtype T, Class C)
{
  
  STP p;
  int result;
  char *getClassStr[] = { "NEWSYM", "FUNC", "PARAM", "VAR", "F_PROT" };
  char *getTypeStr[] = { "void", "int" };
  
  // JSON
  if(afterFuncDefinition == 1){
    if(functionParseState == OUT){
      printf("An error occured. Your syntax isn't recommended.\n");
    }
  }
  
  SymTptr->name = Name;
  for(p = BLKtbl[Level].first; p->name != Name; p++);
  
  if(p >= SymTptr){
    if(T == VOID){
      yyerror("Void is used as a type name");
      T = INT;
    }
    p = MakeEntry(Name, T, C);
    p->loc = Mallocp++;
  }else{
    yyerror("Duplicated declared.");
  }

  if(functionParseState == OUT){
    // same -> a declaration of global variable.
    // do nothing.
  }else{
    // not same -> a declaration of local variable.
    saveVarInfo(Name, T, C);
  }
  
}

STP SymRef(char *Name)
{
  
  STP p;
  if((p = LookUp(Name)) == NULL){
    sprintf(msg, "Ref . to undeclared identifier %s", Name);
    yyerror(msg);
    p = MakeEntry(Name, INT, VAR);
  }
  return p;
  
}

void UndefCheck(void)
{
  
  STP p;
  if(Level > 0)
    yyerror("Block is not properly nested");
  
  for(p = &SymTab[1]; p < SymTptr; p++){
    if(p->type == F_PROT && p->loc > 0){
      sprintf(msg, "Undefined function %s is called", p->name);
      yyerror(msg);
    }
  }
}

void PrintSymEntry(STP Symp)
{
  static char
      *SymC[] = {"NewSym", "Func", "Param", "Var", "P_Type" }, *SymD[] = { "void","int" };

  int result;
  if(functionParseState == IN){
    functionParseState = OUT;
    free(function);
  }else{
    function = (char *) xmalloc(sizeof(Symp->name));
    strcpy(function, Symp->name);
    functionParseState = IN;
  }
}

void PrintAllSymEntry(void)
{
  
  static char 
      *SymC[] = {"newsym", "func", "param", "var", "p_type" }, *SymD[] = { "void","int" };
  
  STP p;
  printf(" { \"global\" : [ \n");
  // JSON
  for(p = &SymTab[1]; p < SymTptr; p++){
    printf("\t\t{ \"name\" : \"%s\", \"type\" : \"%s\", \"class\" : \"%s\" } ",
           p->name, SymD[p->type], SymC[p->class]);
    if(p == (SymTptr - 1)){
      printf("\t]\t ,\n");
    }else{
      printf(",\n");
    }
  }
  printf("\n");
}

void printFuncStack(int array[],int arrayPointer){
  // interesting dummy code.
  STP p;
  int i;
  printf(" { \"path\" : \"");
  for(i = 0; i < arrayPointer; i++){
    // OK
    for(p = &SymTab[1]; p < SymTptr; p++){
      if(p->loc == array[i]){
	// last loop ?
	if(i != (arrayPointer - 1)){
	  if(p->class == FUNC){
	    printf("%s,",p->name);
	  }
	}else{
	  if(p->class == FUNC){
	    printf("%s", p->name);
	  }
	}
      }
    }
  }
  printf("\" , ");
}

void dumpSymTbl(void){
  STP p;
  int i;
  char *c[] = { "newsym", "func", "param", "var", "f_prot" };
  for(p = &SymTab[1]; p < SymTptr; p++){
    printf("name : %s, loc : %d , class : %s \n", p->name, p->loc, c[p->class]);
  }
}

void freeAllMem(void){
  freeUSTbl();
  if(function != NULL){
    free(function);
  }
}

void dumpFuncFrameStr(void){
  // do nothing.
}
