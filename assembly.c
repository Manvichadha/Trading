#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAXSYMS 1024
#define MAXLINE 1024
typedef struct {
    char *name;
    int is_temp;
} Sym;
static Sym syms[MAXSYMS];
static int symcnt = 0;
int ensure_sym(const char *name) {
    for (int i = 0; i < symcnt; ++i) {
        if (strcmp(syms[i].name, name) == 0) return i;
    }
    if (symcnt >= MAXSYMS) {
        fprintf(stderr, "Too many symbols\n");
        exit(1);
    }
    syms[symcnt].name = strdup(name);
    syms[symcnt].is_temp = (name[0]=='t' && isdigit((unsigned char)name[1])); 
    return symcnt++;
}
void trim(char *s) {
    int i = 0, j;
    while (isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s+i, strlen(s+i)+1);
    j = strlen(s)-1;
    while (j >= 0 && isspace((unsigned char)s[j])) s[j--] = '\0';
}
int is_number(const char *s) {
    if (!s || !*s) return 0;
    const char *p = s;
    if (*p=='+' || *p=='-') p++;
    int digits = 0;
    while (*p) { if (!isdigit((unsigned char)*p)) return 0; digits++; p++; }
    return digits>0;
}
int print_load_operand_to_ax(FILE *out, const char *op) {
    if (is_number(op)) {
        fprintf(out, "    mov ax, %s\n", op);
        return 1;
    } else {
        fprintf(out, "    mov ax, [%s]\n", op);
        return 0;
    }
}
void store_ax_to(FILE *out, const char *dest) {
    fprintf(out, "    mov [%s], ax\n", dest);
}
void process_line(FILE *out, char *line) {
    trim(line);
    if (strlen(line) == 0) return;
    int L = strlen(line);
    if (L>0 && line[L-1]==';') line[L-1] = '\0';
    char *eq = strchr(line, '=');
    if (!eq) {
        fprintf(stderr, "Ignoring (no '='): %s\n", line);
        return;
    }
    char lhsbuf[256], rhsbuf[768];
    memset(lhsbuf,0,sizeof(lhsbuf));
    memset(rhsbuf,0,sizeof(rhsbuf));
    strncpy(lhsbuf, line, eq - line);
    lhsbuf[eq-line] = '\0';
    trim(lhsbuf);
    strcpy(rhsbuf, eq + 1);
    trim(rhsbuf);
    if (lhsbuf[0] == '\0' || rhsbuf[0] == '\0') {
        fprintf(stderr, "Malformed line: %s\n", line);
        return;
    }
    ensure_sym(lhsbuf);
    if (rhsbuf[0] == '-' && (rhsbuf[1] == ' ' || rhsbuf[1] == '\0' || !isdigit((unsigned char)rhsbuf[1]))) {
        char *opnd = rhsbuf + 1;
        trim(opnd);
        if (*opnd == '\0') { fprintf(stderr, "Malformed unary minus: %s\n", line); return; }
        ensure_sym(opnd);
        print_load_operand_to_ax(out, opnd);
        fprintf(out, "    neg ax\n");
        store_ax_to(out, lhsbuf);
        return;
    }
    char op1[256], op2[256], opch = 0;
    op1[0]=op2[0]=0;
    int found = 0;
    int len = strlen(rhsbuf);
    int paren = 0;
    for (int i = 0; i < len; ++i) {
        char c = rhsbuf[i];
        if (c == '(') paren++;
        else if (c == ')') paren--;
        else if (paren == 0 && (c == '+' || c == '-')) {
            if (i==0) continue;
            opch = c;
            strncpy(op1, rhsbuf, i); op1[i]='\0';
            strcpy(op2, rhsbuf + i + 1);
            found = 1;
            break;
        }}
    if (!found) {
        paren = 0;
        for (int i = 0; i < len; ++i) {
            char c = rhsbuf[i];
            if (c == '(') paren++;
            else if (c == ')') paren--;
            else if (paren == 0 && (c == '*' || c == '/')) {
                opch = c;
                strncpy(op1, rhsbuf, i); op1[i]='\0';
                strcpy(op2, rhsbuf + i + 1);
                found = 1;
                break;
            }
        }
    }

    if (!found) {
        trim(rhsbuf);
        ensure_sym(rhsbuf);
        print_load_operand_to_ax(out, rhsbuf);
        store_ax_to(out, lhsbuf);
        return;}
    trim(op1); trim(op2);
    if (op1[0]==0 || op2[0]==0) { fprintf(stderr, "Malformed binary op: %s\n", line); return; }
    if (!is_number(op1)) ensure_sym(op1);
    if (!is_number(op2)) ensure_sym(op2);
    if (opch == '+' || opch == '-') {
        print_load_operand_to_ax(out, op1); /* mov ax, op1 */
        if (is_number(op2)) {
            if (opch == '+')
                fprintf(out, "    add ax, %s\n", op2);
            else
                fprintf(out, "    sub ax, %s\n", op2);
        } else {
            if (opch == '+')
                fprintf(out, "    add ax, [%s]\n", op2);
            else
                fprintf(out, "    sub ax, [%s]\n", op2);
        }
        store_ax_to(out, lhsbuf);
        return;
    } else if (opch == '*') {
        print_load_operand_to_ax(out, op1); 
        if (is_number(op2)) {
                      fprintf(out, "    mov bx, %s\n", op2);
            fprintf(out, "    imul bx\n");
        } else {
            fprintf(out, "    imul word ptr [%s]\n", op2);
        }
        store_ax_to(out, lhsbuf);
        return;
    } else if (opch == '/') {
        print_load_operand_to_ax(out, op1); 
        fprintf(out, "    cwd\n");
        if (is_number(op2)) {
            fprintf(out, "    mov bx, %s\n", op2);
            fprintf(out, "    idiv bx\n");
        } else {
            fprintf(out, "    idiv word ptr [%s]\n", op2);
        }
        store_ax_to(out, lhsbuf);
        return;
    } else {
        fprintf(stderr, "Unsupported operator '%c' in line: %s\n", opch, line);
        return;
    }
}
void emit_header(FILE *out) {
    fprintf(out, ".MODEL SMALL\n");
    fprintf(out, ".STACK 100h\n");
    fprintf(out, ".DATA\n");
}
typedef struct LineNode {
    char *s;
    struct LineNode *next;
} LineNode;
int main(int argc, char **argv) {
    char buf[MAXLINE];
    LineNode *head = NULL, *tail = NULL;
    while (fgets(buf, sizeof(buf), stdin)) {
        char linecpy[MAXLINE];
        strncpy(linecpy, buf, sizeof(linecpy)-1); linecpy[sizeof(linecpy)-1]=0;
        trim(linecpy);
        if (linecpy[0] == '\0') continue;
        char *p = linecpy;
        while (*p) {
            if (isalpha((unsigned char)*p) || *p == '_') {
                char tmp[256]; int i = 0;
                while (*p && (isalnum((unsigned char)*p) || *p == '_')) {
                    tmp[i++] = *p++; if (i>=250) break;
                }
                tmp[i]=0;
                ensure_sym(tmp);
            } else p++;
        }
        LineNode *node = malloc(sizeof(LineNode));
        node->s = strdup(linecpy);
        node->next = NULL;
        if (!head) head = tail = node; else { tail->next = node; tail = node; }
    }
    FILE *out = stdout;
    fprintf(out, ".MODEL SMALL\n");
    fprintf(out, ".STACK 100h\n");
    fprintf(out, ".DATA\n");
    for (int i = 0; i < symcnt; ++i) {
        fprintf(out, "%s DW 0\n", syms[i].name);
    }
    fprintf(out, "\n.CODE\n");
    fprintf(out, "MAIN PROC\n");
    fprintf(out, "    mov ax, @DATA\n");
    fprintf(out, "    mov ds, ax\n\n");
    LineNode *cur = head;
    while (cur) {
        process_line(out, cur->s);
        cur = cur->next;
    }
    fprintf(out, "\n    mov ax, 4C00h\n");
    fprintf(out, "    int 21h\n");
    fprintf(out, "MAIN ENDP\n");
    fprintf(out, "END MAIN\n");
    cur = head;
    while (cur) {
        LineNode *n = cur->next;
        free(cur->s);
        free(cur);
        cur = n;
    }
    for (int i = 0; i < symcnt; ++i) free(syms[i].name);
    return 0;
}
