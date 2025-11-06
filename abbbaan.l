%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
static void trim(char *s) {char *p = s;
    while (isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p)+1);
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';}
int recognize_lang2(const char *str) {
    const char *p = str;
    int n = 0;
    if (p[0] != 'a' || p[1] != 'b') return 0;
    p += 2;
    while (p[0] == 'b' && p[1] == 'b' && p[2] == 'a' && p[3] == 'a') {
        n++;
        p += 4;}
    if (!(p[0] == 'b' && p[1] == 'b' && p[2] == 'a')) return 0;
    p += 3;
    for (int i = 0; i < n; ++i) {
        if (!(p[0] == 'b' && p[1] == 'a')) return 0; p += 2;}
    if (*p != '\0') return 0;
    return 1;}
%}
%%
[ \t]+          ;           
\n              { }
[^\n]+          {
                    char buf[2048];
                    strncpy(buf, yytext, sizeof(buf)-1);
                    buf[sizeof(buf)-1] = '\0';
                    trim(buf);
                    if (strlen(buf) == 0) {
                        printf("REJECT\n");
                    } else {
                        int ok = recognize_lang2(buf);
                        printf("%s\n", ok ? "ACCEPT" : "REJECT");
                    }
                }
%%
int main(int argc, char **argv) {
    printf("L2 recognizer: strings of the form ab (bbaa)^n bba (ba)^n  (n >= 0)\n");
    printf("Enter one input string per line (no spaces inside the string).\n\n");
    yylex();
    return 0;
}
