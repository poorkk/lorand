%{
#include <stdio.h>
#include <stdlib.h>
#define YYDEBUG 1
int yylex();
int yyerror(char const *str);
%}
%union{
    int int_value;
    double double_value;
}
%token <int_value> INT_LITERAL
%token <double_value> DOUBLE_LITERAL
%token ADD SUB MUL DIV CR
//%type <int_value> expression term primary_expression
%type <double_value> expression term primary_expression
%%
line_list
    : line
    | line_list line
    ;
line
    : expression CR
    {
        printf(">>%.2lf\n",$1);
    }
expression
    : term
    | expression ADD term
    {
        $$ = $1 + $3;
    }
    | expression SUB term
    {
        $$ = $1 - $3;
    }
    ;
term
    : primary_expression 
    | term MUL primary_expression
    {
        $$ = $1 * $3;
    }
    | term DIV primary_expression
    {
        $$ = $1 / $3;
    }
    ;
primary_expression
    : INT_LITERAL {
        $$ = (double)$1;
    }
    | DOUBLE_LITERAL
    ;
%%
int yyerror(char const *str)
{
    extern char *yytext;
    fprintf(stderr,"parser error near %s,%s\n",yytext,str);
    return 1;
}

int main(void)
{
    extern int yyparse(void);
    extern FILE *yyin;
    yyin=stdin;
    if(yyparse()){
        fprintf(stderr,"Error!\n");
        exit(1);
    }
}