%{
#include <stdio.h> 
#include <string.h> 
int max_len = 0; 
char longest[100]; 
%}
%option noyywrap
%% 
[a-zA-Z]+ {int len = strlen(yytext); if (len > max_len) { max_len = len; strcpy(longest, yytext);} }
.;
\n ;
%%
int main() 
{
yylex();
printf("Longest word: %s (Length: %d)\n", longest, max_len); return 0;
}
