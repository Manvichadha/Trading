%{
#include <stdio.h>
int s=0;
%}
%%
a { if(s==0) s=1; else if(s==1) s=0; else if(s==2) s=3; else if(s==3) s=2; }
b { if(s==0) s=2; else if(s==1) s=3; else if(s==2) s=0; else if(s==3) s=1; }
\n { if(s==1) printf("Accepted\n"); else printf("Rejected\n"); s=0; }
. ;
%%
int main()
{ printf("DFA: Accepts strings with ODD number of 'a's and EVEN number of 'b's.\n");
 yylex();
 return 0;
}
int yywrap()
{
 return 1;
}


//1032
//2301
