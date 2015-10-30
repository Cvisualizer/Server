#ifndef VSM_H_INCLUDED
#define VSM_H_INCLUDED

typedef enum {
	NOP,  ASSIGN, ADD, SUB,   MUL,   DIV,   MOD,   CSIGN,
	AND,  OR,     NOT, COMP,  COPY,  PUSH,  PUSHI, REMOVE,
	POP,  INC,    DEC, SETFR, INCFR, DECFR, JUMP,  BLT,
	BLE,  BEQ,    BNE, BGE,   BGT,   CALL,  RET,   HALT,
	INPUT, OUTPUT } OP;

#define ISEG_SIZE 1000
#define DSEG_SIZE 1000
#define FRAME_BOTTOM (DSEG_SIZE-1)

#define FP 0x01

typedef struct {
	unsigned char Op, Reg;
	int Addr;
} INSTR;

void SetPC(int N);
int PC(void);
int StartVSM(int StartAddr, int TraceSW);

void SetI(OP OPcode,int Flag,int Addr);
//int Bpatch(int Loc, int Addr);
void Bpatch(int Loc, int Addr);
void DumpIseg(int first,int last);
void ExecReport(void);
int getFuncParam(int location);
void printFuncStack(int array[],int arrayPointer);
//void searchGlobalTable(int num);
//void searchLocalTable(int num);

#define Cout(OPcode,Addr) SetI(OPcode,0,Addr)
#define Pout(OPcode) SetI(OPcode,0,0)

extern int DebugSW;

#endif 
