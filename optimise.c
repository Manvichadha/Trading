#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#define MAXLINE 512
static void trim(char *s) {
    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';}
static int is_number(const char *s) {
    if (*s == '\0') return 0;
    const char *p = s;
    if (*p == '-' || *p == '+') p++;
    if (!isdigit((unsigned char)*p)) return 0;
    while (*p) {
        if (!isdigit((unsigned char)*p)) return 0;
        p++;}
    return 1;}
static int64_t to_int64(const char *s) {
    return strtoll(s, NULL, 10);}
static int power_of_two_shift(int64_t n) {
    if (n <= 0) return -1;
    if ((n & (n - 1)) != 0) return -1;
    int k = 0;
    while (n > 1) { n >>= 1; k++; }
    return k;}
static char *optimize_binary(const char *op1, const char *op, const char *op2) {char *out = malloc(256);
    out[0] = '\0';
    int op1_is_num = is_number(op1);
    int op2_is_num = is_number(op2);
    if (op1_is_num && op2_is_num) {
        int64_t a = to_int64(op1);
        int64_t b = to_int64(op2);
        int64_t res = 0;
        int fold_ok = 1;
        if (strcmp(op, "+") == 0) res = a + b;
        else if (strcmp(op, "-") == 0) res = a - b;
        else if (strcmp(op, "*") == 0) res = a * b;
        else if (strcmp(op, "/") == 0) {
            if (b == 0) fold_ok = 0; 
            else res = a / b;}
        else if (strcmp(op, "%") == 0) {
            if (b == 0) fold_ok = 0;
            else res = a % b;
        }
        else if (strcmp(op, "<<") == 0) res = a << b;
        else if (strcmp(op, ">>") == 0) res = a >> b;
        else fold_ok = 0;
        if (fold_ok) {
            snprintf(out, 256, "%" PRId64, res);
            return out;}}
    if (strcmp(op, "+") == 0) {
        if (op2_is_num && to_int64(op2) == 0) { snprintf(out,256,"%s",op1); return out; }
        if (op1_is_num && to_int64(op1) == 0) { snprintf(out,256,"%s",op2); return out; }}
    if (strcmp(op, "-") == 0) {
        if (op2_is_num && to_int64(op2) == 0) { snprintf(out,256,"%s",op1); return out; }
        if (!op1_is_num && !op2_is_num && strcmp(op1, op2) == 0) { snprintf(out,256,"0"); return out; }}
    if (strcmp(op, "*") == 0) {
        if (op2_is_num && to_int64(op2) == 1) { snprintf(out,256,"%s",op1); return out; }
        if (op1_is_num && to_int64(op1) == 1) { snprintf(out,256,"%s",op2); return out; }
        if (op2_is_num && to_int64(op2) == 0) { snprintf(out,256,"0"); return out; }
        if (op1_is_num && to_int64(op1) == 0) { snprintf(out,256,"0"); return out; }}
    if (strcmp(op, "/") == 0) {
        if (op2_is_num && to_int64(op2) == 1) { snprintf(out,256,"%s",op1); return out; }
        if (!op1_is_num && !op2_is_num && strcmp(op1, op2) == 0) { snprintf(out,256,"1"); return out; }
    }
    if (strcmp(op, "%") == 0) {
        if (op2_is_num && to_int64(op2) == 1) { snprintf(out,256,"0"); return out; }
    }
    if (strcmp(op, "*") == 0 && !op1_is_num && op2_is_num) {
        int64_t b = to_int64(op2);
        int k = power_of_two_shift(b);
        if (k >= 0) { snprintf(out,256,"%s << %d", op1, k); return out; }}
    if (strcmp(op, "*") == 0 && op1_is_num && !op2_is_num) {
        int64_t a = to_int64(op1);
        int k = power_of_two_shift(a);
        if (k >= 0) { snprintf(out,256,"%s << %d", op2, k); return out; }
    }
    if (strcmp(op, "/") == 0 && !op1_is_num && op2_is_num) {
        int64_t b = to_int64(op2);
        int k = power_of_two_shift(b);
        if (k >= 0) { snprintf(out,256,"%s >> %d", op1, k); return out; }
    }
    snprintf(out,256,"%s %s %s", op1, op, op2);
    return out;
}
static int find_operator(const char *s, char *opbuf, char *left, char *right) {
    const char *pos = strstr(s, "<<");
    if (!pos) pos = strstr(s, ">>");
    if (pos) {
        int idx = pos - s;
        strncpy(left, s, idx);
        left[idx] = '\0';
        strcpy(opbuf, pos[0] == '<' ? "<<" : ">>");
        strcpy(right, pos+2);
        trim(left); trim(right);
        return 1;}
    int i;
    int n = strlen(s);
    for (i = 0; i < n; ++i) {
        char c = s[i];
        if (c == '+' || c == '*' || c == '/' || c == '%' ) {
            strncpy(left, s, i);
            left[i] = '\0';
            opbuf[0] = c; opbuf[1] = '\0';
            strcpy(right, s + i + 1);
            trim(left); trim(right);
            return 1;}
        if (c == '-') {
            int j = i - 1;
            while (j >= 0 && isspace((unsigned char)s[j])) j--;
            if (j >= 0) {
                strncpy(left, s, i);
                left[i] = '\0';
                opbuf[0] = '-'; opbuf[1] = '\0';
                strcpy(right, s + i + 1);
                trim(left); trim(right);
                return 1;
            }
        }
    }
    left[0] = '\0';
    opbuf[0] = '\0';
    strcpy(right, s);
    trim(right);
    return 0;}
int main(void) {
    char line[MAXLINE];
    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\n")] = 0;
        char tmp[MAXLINE];
        strcpy(tmp, line);
        trim(tmp);
        if (tmp[0] == '\0') { continue; }
        char *eq = strchr(line, '=');
        if (!eq) {
            printf("%s\n", line);
            continue;
        }
        char lhs[MAXLINE], rhs[MAXLINE];
        int lhs_len = eq - line;
        strncpy(lhs, line, lhs_len);
        lhs[lhs_len] = '\0';
        strcpy(rhs, eq + 1);
        trim(lhs); trim(rhs);
        char opbuf[8], left[MAXLINE], right[MAXLINE];
        memset(opbuf,0,sizeof(opbuf));
        left[0] = right[0] = '\0';
        int is_binary = find_operator(rhs, opbuf, left, right);
        if (!is_binary) {
            printf("%s = %s\n", lhs, right);
            continue;
        }
        char *optimized_rhs = optimize_binary(left, opbuf, right);
        printf("%s = %s\n", lhs, optimized_rhs);
        free(optimized_rhs);
    }
    return 0;
}
