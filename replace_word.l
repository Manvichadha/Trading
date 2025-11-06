%{
#include <stdio.h> 
#include <string.h>
char find[50] = "old"; 
char replace[50] = "new"; 
%}
%option noyywrap
%% 
[a-zA-Z]+ {if (strcmp(yytext, find) == 0) printf("%s", replace); else printf("%s", yytext);}
. {printf("%s", yytext);}
\n {printf("\n"); }
%%
int main(int argc, char *argv[]) 
{
if (argc > 2) { strcpy(find, argv[1]); strcpy(replace, argv[2]);} 
yylex();
return 0;
}
