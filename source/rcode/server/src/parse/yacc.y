%{
#include <stdio.h>
#include <stdlib.h>
#define YYDEBUG 1

#define YYSTYPE char *

int yylex();
int yyerror(char const *str);
%}

%token CREATE TABLE INT TEXT
    INSERT INTO
    SELECT FROM 
    UPDATE SET
    DELETE

%token <int_value> Int
%token <string_value> Name

%type <string_value> CreateStmt InsertStmt ParseTree

%%
ParseTree
    : CreateStmt
        {
            $$ = $1;
        }
    | InsertStmt
        {
            $$ = $1;
        }
    ;

CreateStmt
    : CREATE TABLE Name
        {
            char *ret = strdump("create stmt");
            printf("new table '%s'\n", $3);
            $$ = ret;
        }
    ;
InsertStmt
    : INSERT INTO Name
        {
            char *ret = strdump("insert stmt");
            $$ = ret;
        }
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