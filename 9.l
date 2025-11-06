%{
#include <stdio.h> 
#include <string.h>
int count = 0;
char target[50] = "word"; 
%}
%option noyywrap
%%
[a-zA-Z]+ {if (strcmp(yytext, target) == 0) count++;}
.;
\n ;
%%
int main(int argc, char *argv[]) 
{
if (argc > 1) strcpy(target, argv[1]);
yylex();
printf("Frequency of '%s' = %d\n", target, count); return 0;
}