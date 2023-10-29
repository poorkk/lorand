%{
#include <stdio.h>
#include "yacc.tab.h"   /* generate based on .y */

//extern char* yylval;
extern int yyerror(char const *str);

int yywrap(void)
{
    return 1;
}

#define kk_strdup(s) (YYSTYPE)strdup((s))
%}

Char [A-Za-z]
Num [0-9]

Name [Char]+
Int [Num]+

%%

("quit")|("exit") {exit(0);}

CREATE { printf("get [CREATE]\n"); return CREATE; }
TABLE { return TABLE; }
INT|TEXT { printf("dbtype %s\n", yytext); yylval = kk_strdup(yytext); return DbType; }

INSERT { printf("get [INSERT]\n"); return INSERT; }
INTO { return INTO; }
SELECT { return SELECT; }
FROM { return FROM; }
UPDATE { return UPDATE; }
SET { return SET; }
DELETE { return DELETE; }

[a-zA-Z][a-zA-Z0-9]* { yylval = kk_strdup(yytext); printf("Name: %s\n", yytext); return ValName; }
[0-9]+  { yylval = kk_strdup(yytext); printf("Int: %d\n", atoi(yytext)); return ValInt; }

\n  printf("<new line>\n");
[ \t]+  /* ignore whitespace */;
. {printf("<%c>\n", yytext[0]);  return (int) yytext[0]; };

%%