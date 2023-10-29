%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define YYDEBUG 1

// #define YYSTYPE char *
// typedef char * YYSTYPE;

int yylex();
int yyerror(char const *str);
%}

%union {
    int TypeNum;
    char *TypeStr;
}

%token CREATE TABLE INT TEXT
    INSERT INTO
    SELECT FROM 
    UPDATE SET
    DELETE

%token <TypeNum> ValInt
%token <TypeStr> ValName DbType

%type <TypeStr> CreateStmt InsertStmt ParseTree ParseTrees

%%
ParseTrees:
    | ParseTrees ParseTree;

ParseTree
    : CreateStmt
        {
            $$ = $1;
            printf("table name %s\n", $$);
        }
    | InsertStmt
        {
            $$ = $1;
        }
    ;

CreateStmt
    : CREATE ValName '(' ValName DbType ')'
        {
            char *ret = strdup("create stmt");
            printf("new table '%s'\n", $2);
            $$ = ret;
        }
    ;
InsertStmt
    : INSERT ValName
        {
            char *ret = strdup("insert stmt");
            printf("insert value '%s'\n", $2);
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