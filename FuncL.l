%{
#include "VSM.h"
#include "SymTable.h"
#include "y.tab.h"

char *IDentry(char *,int);

%}

ws                [ \t\n]
symbol            [=?!(),:{};]
digit             [0-9]
letter            [a-zA-Z_]
id                {letter}({letter}|{digit})*
other             .

%%

{ws}+                             ;
{symbol}             { return(yytext[0]); }
"+"                  { yylval.Int = ADD; return ADDOP; }
"-"                  { yylval.Int = SUB; return ADDOP; }
"*"                  { yylval.Int = MUL; return MULOP; }
"/"                  { yylval.Int = DIV; return MULOP; }
"%"                  { yylval.Int = MOD; return MULOP; }
"<"                  { yylval.Int = BLT; return RELOP; } 
"<="                 { yylval.Int = BLE; return RELOP; }
"=="                 { yylval.Int = BEQ; return RELOP; }
"!="                 { yylval.Int = BNE; return RELOP; }
">="                 { yylval.Int = BGE; return RELOP; }
">"                  { yylval.Int = BGT; return RELOP; }
"int"                { yylval.Int = INT; return TYPE;  }
"void"               { yylval.Int = VOID;return TYPE;  }
"printf"                                { return WRITE; }
"return"                               { return RETURN;}
"if"                                   { return IF;    }
"else"                                 { return ELSE;  }
{digit}+             { sscanf(yytext,"%d",&yylval.Int) ;      return NUM; }
{id}                 { yylval.Name = IDentry(yytext,yyleng);  return ID;  }
{other}              { fprintf(stderr,"Illegal char '%c' ignored\n",yytext[0]); }

%%
