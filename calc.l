%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
double parse_expr(const char **s);
double parse_term(const char **s);
double parse_power(const char **s);
double parse_factor(const char **s);
void skip_spaces(const char **s);
void skip_spaces(const char **s) {
    while (isspace((unsigned char)**s)) (*s)++;
}
double parse_expr(const char **s) {
    double value = parse_term(s);
    skip_spaces(s);
    while (**s == '+' || **s == '-') {
        char op = *(*s)++;
        double rhs = parse_term(s);
        if (op == '+') value += rhs;
        else value -= rhs;
        skip_spaces(s);
    }
    return value;
}
double parse_term(const char **s) {
    double value = parse_power(s);
    skip_spaces(s);
    while (**s == '*' || **s == '/' || **s == '%') {
        char op = *(*s)++;
        double rhs = parse_power(s);
        if (op == '*') {
            value *= rhs;
        } else if (op == '/') {
            if (rhs == 0.0) {
                fprintf(stderr, "Error: division by zero\n");
                value = INFINITY;
            } else value /= rhs;
        } else { 
            if (rhs == 0.0) {
                fprintf(stderr, "Error: modulo by zero\n");
                value = NAN;
            } else {
                value = fmod(value, rhs);
            }
        }
        skip_spaces(s);
    }
    return value;
}
double parse_power(const char **s) {
    double base = parse_factor(s);
    skip_spaces(s);
    if (**s == '^') {
        (*s)++; 
        double exponent = parse_power(s); 
        base = pow(base, exponent);
    }
    return base;
}
double parse_factor(const char **s) {
    skip_spaces(s);
    if (**s == '+' || **s == '-') {
        char op = *(*s)++;
        double v = parse_factor(s);
        return (op == '-') ? -v : v;
    }

    if (**s == '(') {
        (*s)++; 
        double v = parse_expr(s);
        skip_spaces(s);
        if (**s == ')') (*s)++;
        else fprintf(stderr, "Error: missing ')'\n");
        return v;
    }

    char *endptr;
    double val = strtod(*s, &endptr);
    if (endptr == *s) {
        fprintf(stderr, "Error: expected number at '%s'\n", *s);
        return 0.0;
    }
    *s = endptr;
    return val;
}

double eval_string(const char *text) {
    const char *p = text;
    double result = parse_expr(&p);
    skip_spaces(&p);
    if (*p != '\0') {
        fprintf(stderr, "Warning: leftover characters -> '%s'\n", p);
    }
    return result;
}
%}
%%
[ \t]+          ;              
\n              {  }
[^\n]+          {
                    double r = eval_string(yytext);
                    if (isfinite(r)) {
                        printf("= %.10g\n", r);
                    } else if (isnan(r)) {
                        printf("= NaN\n");
                    } else {
                        printf("= %s\n", (r == INFINITY) ? "Infinity" : "-Infinity");
                    }
                }
%%

int main(int argc, char **argv) {
    printf("Simple LEX calculator (supports + - * / %% ^ and parentheses)\n");
    printf("Note: %% is modulo (uses fmod for floating values), ^ is power.\n");
    printf("Type expressions (one per line). Press Ctrl+C to quit.\n\n");
    yylex();
    return 0;
}
