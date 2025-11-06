#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
bool delimiter(char ch) {
    switch (ch) {
        case ' ': case '\t': case '\n':
        case '+': case '-': case '*': case '/': case '%':
        case ',': case ';':
        case '>': case '<': case '=':
        case '(': case ')': case '[': case ']':
        case '{': case '}':
            return true;
        default:
            return false;
    }
}
bool assop(char ch) { return (ch == '='); }
bool ariop(char ch) { return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%'); }
bool relop(char ch) { return (ch == '>' || ch == '<'); }
bool identifier(const char* token) {
    if (!token || token[0] == '\0') 
        return false;
    if ((token[0] >= '0' && token[0] <= '9') || delimiter(token[0])) 
        return false; 
    return true;
}
bool keyword(const char* token) {
    const char* keywords[] = {
        "if","else","while","do","break","continue","int","double","float",
        "return","char","case","sizeof","long","short","typedef","switch",
        "void","static","default","struct","goto","printf","true","false"};
    int n = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < n; ++i) 
    {
        if (strcmp(token, keywords[i]) == 0) 
        {
            return true;
        }
    }
    return false;
}
bool isInteger(char* token) 
{
    if (!token || *token == '\0') 
        return false; 
    for (int i = 0; token[i]; i++) 
    {
        if (token[i] < '0' || token[i] > '9') 
        {
            return false; 
        }
    }
    return true; 
}
bool isRealNumber(char* token) 
{
    if (!token || *token == '\0') 
        return false; 
    bool decimalFound = false;
    for (int i = 0; token[i]; i++) 
    { 
        if (token[i] == '.') 
        {
            if (decimalFound) 
            {
                return false;
            }
            decimalFound = true;
        } 
        else if (token[i] < '0' || token[i] > '9') 
        {
            return false; 
        }
    }
    return decimalFound; 
}
void analyzeString(const char* input) {
    int len = (int)strlen(input);
    int i = 0;
    while (i < len) {
        char ch = input[i];
        if (delimiter(ch)) {
            if (!isspace((unsigned char)ch)) {
                if (assop(ch)) printf("'%c' IS AN ASSIGNMENT OPERATOR\n", ch);
                else if (ariop(ch)) printf("'%c' IS AN ARITHMETIC OPERATOR\n", ch);
                else if (relop(ch)) printf("'%c' IS A RELATIONAL OPERATOR\n", ch);
                else printf("'%c' IS A DELIMITER\n", ch);
            }
            i++;
        } else {
            int j = i;
            while (j < len && !delimiter(input[j])) j++;
            int toklen = j - i;
            char* token = (char*)malloc(toklen + 1);
            strncpy(token, input + i, toklen);
            token[toklen] = '\0';
            if (keyword(token)) {
                printf("'%s' IS A KEYWORD\n", token);
            } else if (isInteger(token)) {
                printf("'%s' IS AN INTEGER\n", token);
            } else if (isRealNumber(token)) {
                printf("'%s' IS A REAL NUMBER\n", token);
            } else if (identifier(token)) {
                printf("'%s' IS A VALID IDENTIFIER\n", token);
            } else {
                printf("'%s' IS NOT A VALID IDENTIFIER\n", token);
            }
            free(token);
            i = j;
        }
    }
}
int main(void) {
    char codeLine[] = "int a = b + c * d;";
    printf("Input: %s\n\n", codeLine);
    analyzeString(codeLine);
    return 0;
}