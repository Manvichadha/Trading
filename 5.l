%{
#include <stdio.h>
int s=0;
%}
%%
b {if (s == 0) s = 1; else if (s == 1) s = 1; else if (s == 2) s = 3; else if (s == 3) s = 3;}
c {if (s == 0) s = 0; else if (s == 1) s = 2; else if (s == 2) s = 0; else if (s == 3) s = 3;}
a {if (s == 0) s = 0; else if (s == 1) s = 0; else if (s == 2) s = 3; else if (s == 3) s = 3;}
\n {if (s == 3) printf("Accepted(contains \"bca\")\n");else printf("Rejected (does NOT contain \"bca\")\n");s = 0;}
. ;
%%
int main()
{ printf("DFA: Accepts strings over {a,b,c} that contain substring \"bca\".\n");
 yylex();
 return 0;
}
int yywrap()
{
 return 1;
}