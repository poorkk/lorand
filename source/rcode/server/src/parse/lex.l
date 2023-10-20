%{
#include <stdio.h>
#include "yacc.tab.h"   /* generate based on .y */

extern int yyerror(char const *str);
int yywrap(void)
{
    return 1;
}
%}


%%
"+"     return ADD;  /*定义加减乘除，回车返回执行结果的规则*/
"-"     return SUB;
"*"     return MUL;
"/"     return DIV;
"\n"    return CR;

([1-9][0-9]*)|0 {  /*正则表达式，表示接收非0开头的数字或者0*/
    int temp;
    sscanf(yytext,"%d",&temp);
    yylval.int_value=temp;
    return INT_LITERAL;  /*返回整数运算结果*/
}

([0-9]+\.+[0-9]+) {  /*正则表达式，表示接收非0开头的数字或者0*/
    double temp;
    sscanf(yytext,"%lf",&temp);
    yylval.double_value=temp;
    return DOUBLE_LITERAL;  /*返回整数运算结果*/
}

[ \t] ;

("quit")|("exit") {
    exit(0);
}
%%