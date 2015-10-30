#ifndef SYMTABLE_H_INCLUDED
#define SYMTABLE_H_INCLUDED

typedef enum { NEWSYM, FUNC, PARAM, VAR, F_PROT } Class;
typedef enum { VOID, INT } Dtype;
typedef enum { IN, OUT } FuncState;

typedef struct STentry {
  unsigned char type, class;
  unsigned char deflvl, Nparam;
  char *name;
  int loc;
} STentry, *STP;

void OpenBlock(void);
void CloseBlock(void);

void Bpatch(int Loc,int Target);
STP  MakeFuncEntry(char *Name);
void FuncDef(STP Funcp,Dtype T);
void Prototype(STP Funcp,Dtype T);
void EndFdecl(STP Funcp);
void VarDecl(STP Funcp, char *Name,Dtype T,Class C);
STP  SymRef(char *Name);
void UndefCheck(void);
void PrintSymEntry(STP Symp);
void PrintAllSymEntry(void);
void dumpFuncFrameStr(void);
void dumpUSTbl(void);
void freeUSTbl(void);
int PC(void);
void printFuncStack(int array[], int arrayPointer);
void dumpSymTbl(void);
void freeAllMem(void);

#endif /* SYMTABLE_H_INCLUDED */
