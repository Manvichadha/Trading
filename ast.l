%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
typedef enum { NODE_NUM, NODE_OP } NodeType;
typedef struct AST {
    NodeType type;
    double num;           
    char op;                
    struct AST *right;  
    struct AST *left;    
} AST;
AST *new_num(double v) {
    AST *n = (AST*)malloc(sizeof(AST));
    n->type = NODE_NUM;
    n->num = v;
    n->op = 0;
    n->left = n->right = NULL;
    return n;
}
AST *new_op(char op, AST *l, AST *r) {
    AST *n = (AST*)malloc(sizeof(AST));
    n->type = NODE_OP;
    n->op = op;
    n->left = l;
    n->right = r;
    return n;
}
AST *parse_expr(const char **s);
AST *parse_term(const char **s);
AST *parse_factor(const char **s);
void skip_spaces(const char **s);
void skip_spaces(const char **s) {
    while (isspace((unsigned char)**s)) (*s)++;
}
AST *parse_expr(const char **s) {
    skip_spaces(s);
    AST *node = parse_term(s);
    skip_spaces(s);
    while (**s == '+' || **s == '-') {
        char op = *(*s)++;
        AST *rhs = parse_term(s);
        node = new_op(op, node, rhs);
        skip_spaces(s);
    }
    return node;
}
AST *parse_term(const char **s) {
    skip_spaces(s);
    AST *node = parse_factor(s);
    skip_spaces(s);
    while (**s == '*' || **s == '/') {
        char op = *(*s)++;
        AST *rhs = parse_factor(s);
        node = new_op(op, node, rhs);
        skip_spaces(s);
    }
    return node;
}
AST *parse_factor(const char **s) {
    skip_spaces(s);
    if (**s == '+' || **s == '-') {
        char op = *(*s)++;
        AST *child = parse_factor(s);
        if (op == '-') {
            return new_op('u', child, NULL);
        } else {
            return new_op('U', child, NULL);
        }
    }
    if (**s == '(') {
        (*s)++;
        AST *node = parse_expr(s);
        skip_spaces(s);
        if (**s == ')') (*s)++;
        else fprintf(stderr, "Error: missing ')'\n");
        return node;
    }
    char *endptr;
    double v = strtod(*s, &endptr);
    if (endptr == *s) {
        fprintf(stderr, "Error: expected number at \"%s\"\n", *s);
        return new_num(0.0);
    }
    *s = endptr;
    return new_num(v);
}
void print_postfix(AST *n) {
    if (!n) return;
    if (n->type == NODE_NUM) {
        printf("%.10g", n->num);
    } else if (n->type == NODE_OP) {
        if (n->right == NULL) {
            print_postfix(n->left);
            if (n->op == 'u') printf(" u");
            else if (n->op == 'U') printf(" U");
            else printf(" %c", n->op);
            return;
        }
        print_postfix(n->left);
        printf(" ");
        print_postfix(n->right);
        printf(" %c", n->op);
    }
}
void free_ast(AST *n) {
    if (!n) return;
    if (n->left) free_ast(n->left);
    if (n->right) free_ast(n->right);
    free(n);
}
void infix_to_postfix_and_print(const char *text) {
    const char *p = text;
    AST *root = parse_expr(&p);
    skip_spaces(&p);
    if (*p != '\0') {
        fprintf(stderr, "Warning: leftover characters -> '%s'\n", p);
    }
    print_postfix(root);
    printf("\n");
    free_ast(root);
}

%}

%%

[ \t]+          ;              
\n              {  }
[^\n]+          {
                    infix_to_postfix_and_print(yytext);
                }

%%

int main(int argc, char **argv) {
    printf("Infix -> Postfix (AST) using LEX only\n");
    printf("Enter one infix expression per line. Supports + - * /, parentheses, unary +/-.\n");
    printf("Unary minus in postfix is printed as 'u' (so -5 -> \"5 u\").\n");
    printf("Example: (2 + 3) * -4  ->  \"2 3 + 4 u *\"\n\n");
    yylex();
    return 0;
}
