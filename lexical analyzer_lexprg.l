%{ 
#include <stdio.h>
#include <string.h>
char *keywords[] = {"int", "float", "if", "else", "while", "return"}; 
int isKeyword(char *word) 
{ for (int i = 0; i < 6; i++)
        if (strcmp(word, keywords[i]) == 0) 
            return 1; 
    return 0;
}
%}
%option noyywrap
%%
[ \t\n]
"=="|"!="|"<="|">="|"<"|">"|"="|"+"|"-"|"*"|"/" {printf("OPERATOR: %s\n", yytext); } 
[0-9]+ { printf("INTEGER: %s\n", yytext); }
[0-9]+\.[0-9]+ { printf("REAL NUMBER: %s\n", yytext); } 
[a-zA-Z_][a-zA-Z0-9_]* {if (isKeyword(yytext)) printf("KEYWORD: %s\n", yytext); else printf("IDENTIFIER: %s\n", yytext);}
. {printf("SPECIAL SYMBOL: %s\n", yytext);} 
%%
int main() 
{
yylex();
return 0; 
}
