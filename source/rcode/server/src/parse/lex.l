%{
#include <stdio.h>
#include "yacc.tab.h"   /* generate based on .y */

//extern char* yylval;
extern int yyerror(char const *str);

int yywrap(void)
{
    return 1;
}
%}

Char [A-Za-z]
Num [0-9]

Name [Char]+
Int [Num]+

%%

CREATE { return CREATE; }
TABLE { return TABLE; }
INT { return INT; }
TEXT {  return TEXT; }

INSERT { return INSERT; }
INTO { return INTO; }
SELECT { return SELECT; }
FROM { return FROM; }
UPDATE { return UPDATE; }
SET { return SET; }
DELETE { return DELETE; }

Name { yylval = strdup(yytext); return Name; }
Int  { yylval = strdup(yytext); return Int; }

[ \t] ;

("quit")|("exit") {
    exit(0);
}
%%