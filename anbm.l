%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
static void trim(char *s) {
    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p)+1);
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
}
int recognize_lang1(const char *str) {
    const char *p = str;
    int count_a = 0, count_b = 0;
    while (*p == 'a') { count_a++; p++; }
    while (*p == 'b') { count_b++; p++; }
    if (*p != '\0') return 0;
    return (count_a != count_b);
}
%}
%%
[ \t]+          ;
\n              { }
[^\n]+          {
                    char buf[1024];
                    strncpy(buf, yytext, sizeof(buf)-1);
                    buf[sizeof(buf)-1] = '\0';
                    trim(buf);
                    if (strlen(buf) == 0) {
                        printf("REJECT\n");
                    } else {
                        int ok = recognize_lang1(buf);
                        printf("%s\n", ok ? "ACCEPT" : "REJECT");
                    }
                }
%%
int main(int argc, char **argv) {
    printf("L1 recognizer: strings of form a^n b^m with m != n\n");
    printf("Enter one input string per line (only 'a' and 'b' are meaningful).\n\n");
    yylex();
    return 0;
}
