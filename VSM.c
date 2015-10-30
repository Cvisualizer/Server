#include <stdio.h>
#include <limits.h>
#include "VSM.h"

int DebugSW = 0;
static int Pctr = 0, SP = 0, Freg = 0;
static int InsCount = 0, MaxSD = 0, MinFR = DSEG_SIZE, MaxPC = 0, CallC = 0;

/* new settings for function.
 * 
 * ExecutionCount     : the count of executed instruction codes.
 * FunctionStackCount : the state of function stack.
 * EXEC_LIMIT         : the count of executed instruction codes to avoid infinite loop.
 * FUNC_STACK_LIMIT   : the limit of function accumulation for mainly recursive call.
 */
static int ExecutionCount = 0, FunctionStackCount = 0;
#define EXEC_LIMIT 100000
#define FUNC_STACK_LIMIT 50

/* 
 * variables for software visualization.
 * 
 * maxVarValue    : the "max" value which some variable number possessed.
 * minVarValue    : the "min" value which some variable number possessed.
 * currentDepth   : the current depth of function.
 * maxDepth       : the max depth of function.
 * useAssignFlag  : did user use assign instruction code ? (1/0)
 */

static int maxVarValue = INT_MIN, minVarValue = INT_MAX;
static int currentDepth = 0, maxDepth = 0, useAssignFlag = 0;
static int FuncStack[FUNC_STACK_LIMIT];
int FuncStackPointer = 0;

static INSTR Iseg[ISEG_SIZE];
static int Dseg[DSEG_SIZE];

#define STACK_SIZE 100
static int Stack[STACK_SIZE];

/*
 * This variables can visualize frame movements of functions.
 */

char *func;
int param;
int loop, addrLoc;
#define DYNAMIC_STACK_SIZE 60
typedef struct {
  char *var;
} DynamicStack[DYNAMIC_STACK_SIZE];

char *Scode[] = {
  "Nop"    ," ="    , " +"    , " -"     , " *"   , " /"  ,
  " %"     , " -'"  , "and"   , "or"     , "not"  , "comp",
  "copy"   , "push" , "push-i", "remove" , "pop"  ,  " ++",
  " --"    , "setFR", "++FR"  , "--FR"   , "jump" , "<0 ?",
  "<=0 ?"  , "==0 ?", "!=0 ?" , ">=0 ?"  , ">0 ?" , "call",
  "return" , "halt" , "input" , "output" 
} ;

static void PrintIns(int loc)
{
  int op;
  op = Iseg[loc].Op;
  printf("%5d %-8s ", loc, Scode[op]);
  switch(op){
    case PUSH :case PUSHI:  case POP: case SETFR: case INCFR:
    case DECFR:case JUMP :  case BLT: case BLE  : case BEQ  :
    case BNE  :case BGE  :  case BGT: case CALL :
      printf("%d - %d", Iseg[loc].Addr, Iseg[loc].Reg);
      break;
    default:
      printf("%10c", ' '); 
  }
}

void SetPC(int Addr)
{
  Pctr = Addr;
}
int PC(void)
{
  return Pctr;
}
void SetI(OP OpCode,int F,int Addr)
{

  Iseg[Pctr].Op = OpCode;
  Iseg[Pctr].Reg = F;
  Iseg[Pctr].Addr = Addr;
  
  if(DebugSW){
    PrintIns(Pctr);
  }
  
  if(++Pctr > MaxPC){
    MaxPC = Pctr;
  }
}

void Bpatch(int Loc,int Target)
{
  
  while(Loc >= 0){
    int p;
    if((p = Iseg[Loc].Addr) == Loc){
      printf("(Bpatch) Trying to rewrite self.");
      return;
    }
    Iseg[Loc].Addr = Target;
    Loc = p;

  }
}

#define BINOP(OP) { Stack[SP-1] = Stack[SP-1] OP Stack[SP]; SP--; }

int StartVSM(int StartAddr,int TraceSW){
  
  int addr, op;
  Pctr = StartAddr;
  SP = Freg = 0;

  printf("[");
  while(1)  {
    
    if(SP >= STACK_SIZE || SP < 0){
      fprintf(stderr, " Illegal Stack Pointer : %d.\n", SP);
      return -1;
    }
    op = Iseg[Pctr].Op;
    addr = Iseg[Pctr].Addr;
    ExecutionCount = ExecutionCount + 1;
    if(EXEC_LIMIT < ExecutionCount){
      printf("\t\t{ \"error \" : \"breakexecutioncode\"  }  ] } \n");
      return -3;
    }
    
    if(Iseg[Pctr++].Reg & 0x01){
      addr += Freg;
    }
    
    InsCount++;
    if(SP > MaxSD){
      MaxSD = SP;
    }
    
    if(TraceSW){
      // -*- the contents of Symbol Table -*-
      // PrintIns(Pctr - 1);
      // -*- Instruction codes -*-
      printf("%15d %5d %12d\n", addr, SP, Stack[SP]);
    }
    
    switch(op){
      case NOP   :                                 continue;
      case ASSIGN: addr = Stack[--SP];
	useAssignFlag = 1;
	// the processings related to "info" key.
	if(maxVarValue <= Stack[SP + 1]){
	  maxVarValue = Stack[SP + 1];
	}
	if(minVarValue >= Stack[SP + 1]){
	  minVarValue = Stack[SP + 1];
	}
	//JSON.
	printf("\t\t{ \"assign\" : ");
	printFuncStack(FuncStack,FuncStackPointer);
	printf(" \"first\" : %d , ", (Freg + 1));
	printf(" \"addr\" : %d , ", addr);
	printf(" \"value\" : %d ,", Stack[SP + 1]);
	if((addr - Freg) > 0){
	  printf(" \"global\" : false ");
	  // print the name of local variable.
	}else{
	  printf(" \"global\" : true ");
	  // print the name of global variable.
	}
	printf("   }   },\n");
	Dseg[addr] = Stack[SP] = Stack[SP + 1];    continue;
      case ADD : BINOP(+);                         continue;
      case SUB : BINOP(-);                         continue;
      case MUL : BINOP(*);                         continue;
      case DIV : if(Stack[SP] == 0){
                    printf("(DIV) Zero divider detected.\n");
                    return -2;
                 }
                 BINOP(/);                         continue;
      case MOD : if(Stack[SP] == 0){
                    printf("(MOD) Zero divider detected !\n");
                    return -2;
                 }
                 BINOP(%);                        continue;
      case CSIGN: Stack[SP] = -Stack[SP];         continue;
      case AND :
        printf("VSM caught AND op.\n");
        return -1;
        continue;
      case OR  :
        printf("VSM caught OR op.\n");
        return -1;
        continue;
      case NOT :  Stack[SP] = !Stack[SP];          continue;
      case COMP:  Stack[SP-1] = Stack[SP-1] > Stack[SP] ? 1: 
        Stack[SP-1] < Stack[SP] ? -1 : 0;
        SP--;                                      continue;
      case COPY: ++SP; Stack[SP] = Stack[SP-1];    continue;
	// PUSH : "reference"
      case PUSH: Stack[++SP] = Dseg[addr];
	// the next instruction code is 'INCFR' ?
	if(Iseg[Pctr].Op == INCFR){
	  // this 'PUSH' is related to "processing of function"
	  // There's no need to print JSON code.
	  continue;
	}
	// addr : relative address.
	// Freg : base address.
	printf("\t\t{ \"ref\"    :  { \"first\" : %d, \"addr\" : %d, ", Freg + 1, addr);
	if((addr - Freg) > 0){
	  printf(" \"global\" : false  }   } ,\n");
	}else{
	  printf(" \"global\" : true  }   } ,\n");
	}
	continue;
      case PUSHI: Stack[++SP] = addr;              continue;
      case REMOVE:              --SP;              continue;
      case POP:  Dseg[addr] = Stack[SP--];         continue;
      case INC:  Stack[SP] = ++Stack[SP];          continue;
      case DEC:  Stack[SP] = --Stack[SP];          continue;
      case SETFR: Freg = addr;                     continue;
      case INCFR: if((Freg += addr) >= DSEG_SIZE){
          printf("Freg overflow at loc.%d\n", Pctr - 1);
          return -3;
        }                                    continue;
      case DECFR: Freg -= addr;
        if(Freg < MinFR) MinFR = Freg;             continue;
      case JUMP:Pctr = addr;                       continue;
      case BLT: if(Stack[SP--] <  0) Pctr = addr;  continue;
      case BLE: if(Stack[SP--] <= 0) Pctr = addr;  continue;
      case BEQ: if(Stack[SP--] == 0) Pctr = addr;  continue;
      case BNE: if(Stack[SP--] != 0) Pctr = addr;  continue;
      case BGE: if(Stack[SP--] >= 0) Pctr = addr;  continue;
      case BGT: if(Stack[SP--] >  0) Pctr = addr;  continue;
      case CALL:
	// "info" : key
	currentDepth++;
	if(maxDepth <= currentDepth){
	  maxDepth = currentDepth;
	}
	Stack[++SP] = Pctr; Pctr = addr; CallC++;
	// standard JSON output.
	if(CallC == 1){
	  // the first call of function "main"
	  printf("\t\t{ \"call\"   :  { \"function\"  : \"main\" , \"param\" : [ ]  }  } ,\n");
	}else{
	  param = getFuncParam(addr);
	  printf(" \"param\" : [ ");
	  if(param == 0){
	    printf(" ] } }, \n");
	  }else{
	    addrLoc = SP - 1;
	    for(loop = 1; loop <= param; loop++){
	      // "info" key -----
	      if(minVarValue >= Stack[addrLoc]){
		minVarValue = Stack[addrLoc];
	      }
	      if(maxVarValue <= Stack[addrLoc]){
		maxVarValue = Stack[addrLoc];
	      }
	      // did user call function which contains parameters ?
	      // Yes.
	      useAssignFlag = 1;
	      // print JSON output.
	      printf(" %d ", Stack[addrLoc--]);
	      if(loop != param){
		printf(",");
	      }
	    }
	    printf(" ]  }   },\n");
	  }
	}
	// get ready for "ASSIGN" Instruction Code by coding Stack.
	// get function locations by searching the location at object code in Symbol Table
	if(FuncStackPointer < FUNC_STACK_LIMIT){
	  FuncStack[FuncStackPointer++] = addr;
	}else{
	  printf("\t\t{ \"error\" : \"funcstackoverflow\"  }  ] } \n");	  
	  return -3;
	}
                                                   continue;
      case RET : Pctr = Stack[SP--];
	// "info" key's value.
	currentDepth--;
	// dec the value controlling function stack.
	FuncStackPointer = FuncStackPointer - 1;
	// JSON 
	printf("\t\t{ \"return\" : %d }, \n", Stack[SP]);
	continue;
      case HALT:
	//printf("\t\t{ \"halt\" : 0  }  ] } \n");
	printf("\t\t{ \"halt\"   : 0  }  ]    , \n");
	printf(" \"info\" :       { \"depth\" : %d, \"call\" : %d, \"max\" : %d,  \"min\" : %d, \"assign\" : %s   }    }\n",
	       maxDepth, CallC, maxVarValue, minVarValue, (useAssignFlag == 1 ? "true" : "false"));
                                                   return 0;
      case INPUT : scanf("%d", &Dseg[Stack[SP--]]);
                                                   continue;
    case OUTPUT:
      printf("\t\t{ \"printf\" : %d } ,\n", Stack[SP--]);
                                                   continue;
      default:
        printf(" There's no case which matches %d .", op);
        printf(" Illegal Op. code at localtion [%d] !\n", Pctr);
        return -4;
    }
  }
}

void DumpIseg(int first, int last)
{
  //printf("\n -*- Contents of Instruction Segment -*- \n");
  for(;first <= last; first++){
    PrintIns(first);
    printf("\n");
  }
  //printf("\n"); 
}

void ExecReport(void)
{
  /*
  printf("\n Object Code Size:%10d ins.\n",MaxPC);
  printf("Max StackDepth : %10d",DSEG_SIZE - MinFR);
  printf("Function calls : %10d times",CallC);
  printf("Execution Count: %10d ins. \n\n",InsCount);
  */
}
