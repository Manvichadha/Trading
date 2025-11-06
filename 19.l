%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
typedef enum {
    TOK_NONE,
    TOK_ID,
    TOK_NUM,
    TOK_OP,
    TOK_LP,
    TOK_RP,
    TOK_ASSIGN,
    TOK_SEMI,
    TOK_END
} TokType;
typedef struct {
    TokType type;
    char op;
    char *text;
} Token;
typedef struct {
    Token *data;
    int len;
    int cap;
} TokVec;
static void tokvec_init(TokVec *v) { v->data = NULL; v->len = v->cap = 0; }
static void tokvec_push(TokVec *v, Token t) {
    if (v->len == v->cap) {v->cap = v->cap ? v->cap*2 : 8;  v->data = realloc(v->data, v->cap * sizeof(Token));}
    v->data[v->len++] = t;}
static void tokvec_free(TokVec *v) {
    for (int i=0;i<v->len;i++) if (v->data[i].text) free(v->data[i].text);
    free(v->data); v->data = NULL; v->len = v->cap = 0;}
static char *strdup_range(const char *s, int a, int b) {
    int n = b - a;
    char *r = malloc(n+1);
    memcpy(r, s+a, n);
    r[n] = '\0';
    return r;}
static int is_id_start(char c) { return isalpha((unsigned char)c) || c == '_'; }
static int is_id_char(char c) { return isalnum((unsigned char)c) || c == '_'; }
static void tokenize(const char *s, TokVec *out) {
    tokvec_init(out);
    int i = 0;
    int n = strlen(s);
    TokType prev = TOK_NONE;
    while (i < n) {
        char c = s[i];
        if (isspace((unsigned char)c)) { i++; continue; }
        if (is_id_start(c)) {
            int start = i;
            i++;
            while (i < n && is_id_char(s[i])) i++;
            Token t; t.type = TOK_ID; t.op = 0; t.text = strdup_range(s, start, i);
            tokvec_push(out, t);
            prev = TOK_ID; continue;}
        if (isdigit((unsigned char)c) || (c=='.' && i+1<n && isdigit((unsigned char)s[i+1]))) {
            int start = i;
            if (s[i] == '.') i++;
            while (i < n && isdigit((unsigned char)s[i])) i++;
            if (i < n && s[i] == '.') {
                i++;
                while (i < n && isdigit((unsigned char)s[i])) i++;}
            if (i < n && (s[i]=='e' || s[i]=='E')) {
                i++;
                if (i < n && (s[i]=='+' || s[i]=='-')) i++;
                while (i < n && isdigit((unsigned char)s[i])) i++;}
            Token t; t.type = TOK_NUM; t.op = 0; t.text = strdup_range(s, start, i);
            tokvec_push(out, t);
            prev = TOK_NUM; continue;}
        if (c == '(') { Token t={TOK_LP,0,NULL}; tokvec_push(out,t); i++; prev = TOK_LP; continue; }
        if (c == ')') { Token t={TOK_RP,0,NULL}; tokvec_push(out,t); i++; prev = TOK_RP; continue; }
        if (c == ';') { Token t={TOK_SEMI,0,NULL}; tokvec_push(out,t); i++; prev = TOK_SEMI; continue; }
        if (c == '=') { Token t={TOK_ASSIGN,0,NULL}; tokvec_push(out,t); i++; prev = TOK_ASSIGN; continue; }
        if (c=='+' || c=='-' || c=='*' || c=='/' || c=='^') {
            int unary = 0;
            if (c == '+' || c == '-') {
                if (prev == TOK_NONE || prev == TOK_LP || prev == TOK_OP || prev == TOK_ASSIGN) unary = 1;
            }
            Token t;
            t.type = TOK_OP;
            t.text = NULL;
            if (unary) {
                t.op = (c=='-') ? 'u' : 'U';
            } else {
                t.op = c;
            }
            tokvec_push(out, t);
            i++;
            prev = TOK_OP;
            continue;
        }
        {
            Token t; t.type = TOK_ID; t.op = 0;
            t.text = strdup_range(s,i,i+1);
            tokvec_push(out,t);
            i++; prev = TOK_ID;
        }
    }
}
static int prec(char op) {
    if (op == 'u' || op == 'U') return 4;
    if (op == '^') return 3;
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}
static int is_right_assoc(char op) {
    if (op == '^' || op == 'u' || op == 'U') return 1;
    return 0;
}
typedef struct {
    char *data; int len, cap;
} CharStack;
static void cs_init(CharStack *s){ s->data=NULL; s->len=s->cap=0; }
static void cs_push(CharStack *s, char c){ if(s->len==s->cap){ s->cap=s->cap? s->cap*2:8; s->data=realloc(s->data,s->cap); } s->data[s->len++]=c; }
static char cs_top(CharStack *s){ return s->len? s->data[s->len-1] : 0; }
static char cs_pop(CharStack *s){ return s->len? s->data[--s->len] : 0; }
static int cs_empty(CharStack *s){ return s->len==0; }
static void cs_free(CharStack *s){ free(s->data); s->data=NULL; s->len=s->cap=0; }
static int infix_to_postfix(const TokVec *in, TokVec *out) {
    tokvec_init(out);
    CharStack opstack; cs_init(&opstack);

    for (int i=0;i<in->len;i++) {
        Token t = in->data[i];
        if (t.type == TOK_NUM || t.type == TOK_ID) {
            Token copy = t;
            copy.text = t.text ? strdup(t.text) : NULL;
            tokvec_push(out, copy);
            continue;
        }
        if (t.type == TOK_OP) {
            char o1 = t.op;
            while (!cs_empty(&opstack)) {
                char o2 = cs_top(&opstack);
                if (o2 == '(') break;
                int p1 = prec(o1), p2 = prec(o2);
                if ((!is_right_assoc(o1) && p1 <= p2) || (is_right_assoc(o1) && p1 < p2)) {
                    cs_pop(&opstack);
                    Token opToken; opToken.type = TOK_OP; opToken.op = o2; opToken.text = NULL;
                    tokvec_push(out, opToken);
                    continue;
                }
                break;
            }
            cs_push(&opstack, o1);
            continue;
        }
        if (t.type == TOK_LP) {
            cs_push(&opstack, '(');
            continue;
        }
        if (t.type == TOK_RP) {
            int found = 0;
            while (!cs_empty(&opstack)) {
                char c = cs_pop(&opstack);
                if (c == '(') { found = 1; break; }
                Token opToken; opToken.type = TOK_OP; opToken.op = c; opToken.text = NULL;
                tokvec_push(out, opToken);
            }
            if (!found) {
                cs_free(&opstack);
                tokvec_free(out);
                return 0;
            }
            continue;
        }
    }
    while (!cs_empty(&opstack)) {
        char c = cs_pop(&opstack);
        if (c == '(' || c == ')') { cs_free(&opstack); tokvec_free(out); return 0; }
        Token opToken; opToken.type = TOK_OP; opToken.op = c; opToken.text = NULL;
        tokvec_push(out, opToken);
    }
    cs_free(&opstack);
    return 1;
}
typedef struct {
    char **data; int len, cap;
} StrStack;
static void ss_init(StrStack *s){ s->data=NULL; s->len=s->cap=0; }
static void ss_push(StrStack *s, char *x){ if(s->len==s->cap){ s->cap=s->cap? s->cap*2:8; s->data=realloc(s->data, s->cap * sizeof(char*)); } s->data[s->len++]=x; }
static char *ss_pop(StrStack *s){ return s->len? s->data[--s->len] : NULL; }
static void ss_free(StrStack *s){ for(int i=0;i<s->len;i++) free(s->data[i]); free(s->data); s->data=NULL; s->len=s->cap=0; }
static int temp_counter = 0;
static char *new_temp() {
    temp_counter++;
    char buf[32];
    snprintf(buf, sizeof(buf), "t%d", temp_counter);
    return strdup(buf);
}
static int postfix_to_tac(const TokVec *postfix, const char *lhs_var) {
    StrStack st; ss_init(&st);
    for (int i=0;i<postfix->len;i++) {
        Token t = postfix->data[i];
        if (t.type == TOK_NUM || t.type == TOK_ID) {
            ss_push(&st, strdup(t.text));
            continue;
        }
        if (t.type == TOK_OP) {
            if (t.op == 'u' || t.op == 'U') {
                char *a = ss_pop(&st);
                if (!a) { fprintf(stderr, "Error: Unary operand missing\n"); ss_free(&st); return 0; }
                char *temp = new_temp();
                if (t.op == 'u') printf("%s = - %s\n", temp, a);
                else printf("%s = + %s\n", temp, a);
                free(a);
                ss_push(&st, temp);
                continue;
            } else {
                char *right = ss_pop(&st);
                char *left  = ss_pop(&st);
                if (!left || !right) { fprintf(stderr, "Error:Binary operands missing\n"); if (left) free(left); if (right) free(right); ss_free(&st); return 0; }
                char *temp = new_temp();
                printf("%s = %s %c %s\n", temp, left, t.op, right);
                free(left); free(right);
                ss_push(&st, temp);
            }
        }
    }
    char *result = ss_pop(&st);
    if (!result) { fprintf(stderr, "Error: empty expression\n"); ss_free(&st); return 0; }
    if (lhs_var) {
        printf("%s = %s\n", lhs_var, result);
    } else {
        printf("# result: %s\n", result);
    }
    free(result);
    while (st.len) { char *x = ss_pop(&st); free(x); }
    ss_free(&st);
    return 1;
}

static void process_line(const char *line) {
    TokVec toks;
    tokenize(line, &toks);
    if (toks.len == 0) { tokvec_free(&toks); return; }

    if (toks.len >= 3 && toks.data[0].type == TOK_ID && toks.data[1].type == TOK_ASSIGN) {
        char *lhs = strdup(toks.data[0].text);
        int start = 2;
        int end = toks.len;
        if (toks.data[end-1].type == TOK_SEMI) end--;
        TokVec expr; tokvec_init(&expr);
        for (int i = start; i < end; ++i) {
            Token cp = toks.data[i];
            if (cp.text) cp.text = strdup(cp.text);
            tokvec_push(&expr, cp);
        }
        TokVec postfix;
        int ok = infix_to_postfix(&expr, &postfix);
        if (!ok) {
            fprintf(stderr, "Error: syntax error (parentheses mismatch or invalid expr)\n");
        } else {
            postfix_to_tac(&postfix, lhs);
        }
        tokvec_free(&expr);
        tokvec_free(&postfix);
        free(lhs);
    } else {
        fprintf(stderr, "Error: only assignments of form `id = expression ;` supported. Got: \"%s\"\n", line);
    }
    tokvec_free(&toks);
}
%}
%%
[ \t]+          ;
\n              { }
[^\n]+          {
                    char buf[4096];
                    strncpy(buf, yytext, sizeof(buf)-1);
                    buf[sizeof(buf)-1] = '\0';
                    process_line(buf);
                }
%%
int main(int argc, char **argv) {
    printf("Three-Address Code generator\n");
    printf("Enter statements, like x = a + b * (c - 2);\n");
    yylex();
    return 0;
}
