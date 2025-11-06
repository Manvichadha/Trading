#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define MAX_PROD 20
#define MAX_SYMBOL_LEN 10
#define MAX_RHS_LEN 20
#define MAX_NON_TERMINALS 10
#define MAX_TERMINALS 20
#define MAX_FIRST_FOLLOW 20
#define MAX_PARSE_ENTRY_LEN 200
typedef struct {
    char lhs[MAX_SYMBOL_LEN];
    char rhs[MAX_RHS_LEN][MAX_SYMBOL_LEN];
    int rhsCount;
} Production;
Production productions[MAX_PROD];
int nProductions;
char nonTerminals[MAX_NON_TERMINALS][MAX_SYMBOL_LEN];
int nNonTerminals;
char terminals[MAX_TERMINALS][MAX_SYMBOL_LEN];
int nTerminals;
char FIRST[MAX_NON_TERMINALS][MAX_FIRST_FOLLOW][MAX_SYMBOL_LEN];
int firstCount[MAX_NON_TERMINALS];
char FOLLOW[MAX_NON_TERMINALS][MAX_FIRST_FOLLOW][MAX_SYMBOL_LEN];
int followCount[MAX_NON_TERMINALS];
char parseTable[MAX_NON_TERMINALS][MAX_TERMINALS][MAX_PARSE_ENTRY_LEN];
int isLL1 = 1;  
int isNonTerminal(char *symbol) {
    for (int i = 0; i < nNonTerminals; i++) {
        if (strcmp(nonTerminals[i], symbol) == 0)
            return 1;
    }
    return 0;
}
int isTerminal(char *symbol) {
    for (int i = 0; i < nTerminals; i++) {
        if (strcmp(terminals[i], symbol) == 0)
            return 1;
    }
    return 0;
}
int indexOfNT(char *symbol) {
    for (int i = 0; i < nNonTerminals; i++) {
        if (strcmp(nonTerminals[i], symbol) == 0)
            return i;
    }
    return -1;
}
int indexOfT(char *symbol) {
    for (int i = 0; i < nTerminals; i++) {
        if (strcmp(terminals[i], symbol) == 0)
            return i;
    }
    return -1;
}
void addToSet(char set[][MAX_SYMBOL_LEN], int *count, char *symbol) {
    for (int i = 0; i < *count; i++) {
        if (strcmp(set[i], symbol) == 0) return;
    }
    strcpy(set[(*count)++], symbol);
}
int containsEpsilon(char set[][MAX_SYMBOL_LEN], int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(set[i], "epsilon") == 0)
            return 1;
    }
    return 0;
}
void addAll(char dest[][MAX_SYMBOL_LEN], int *destCount, char src[][MAX_SYMBOL_LEN], int srcCount) {
    for (int i = 0; i < srcCount; i++) {
        addToSet(dest, destCount, src[i]);
    }
}
void inputGrammar() {
    printf("Enter number of productions: ");
    scanf("%d", &nProductions);
    getchar();
    nNonTerminals = 0;
    nTerminals = 0;
    printf("Enter productions (Format: A=alpha with space-separated RHS symbols, use 'epsilon' for empty):\n");
    for (int i = 0; i < nProductions; i++) {
        char line[100];
        fgets(line, sizeof(line), stdin);
        char *eqPos = strchr(line, '=');
        if (!eqPos) {
            printf("Invalid production format.\n");
            i--;
            continue;
        }
        char lhs[MAX_SYMBOL_LEN];
        strncpy(lhs, line, eqPos - line);
        lhs[eqPos - line] = '\0';
        strcpy(productions[i].lhs, lhs);
        if (!isNonTerminal(lhs)) {
            strcpy(nonTerminals[nNonTerminals++], lhs);
        }
        productions[i].rhsCount = 0;
        char *rhsStr = eqPos + 1;
        char *token = strtok(rhsStr, " \t\n");
        while (token) {
            strcpy(productions[i].rhs[productions[i].rhsCount++], token);
            token = strtok(NULL, " \t\n");
        }
    }
    for (int i = 0; i < nProductions; i++) {
        for (int j = 0; j < productions[i].rhsCount; j++) {
            char *sym = productions[i].rhs[j];
            if (!isNonTerminal(sym) && strcmp(sym, "epsilon") != 0) {
                if (!isTerminal(sym)) {
                    strcpy(terminals[nTerminals++], sym);
                }
            }
        }
    }
    strcpy(terminals[nTerminals++], "$");
}
void computeFirst() {
    for (int i = 0; i < nNonTerminals; i++) firstCount[i] = 0;
    int changed;
    do {
        changed = 0;
        for (int i = 0; i < nProductions; i++) {
            int lhsIdx = indexOfNT(productions[i].lhs);
            if (lhsIdx == -1) continue;
            if (productions[i].rhsCount == 1 && strcmp(productions[i].rhs[0], "epsilon") == 0) {
                int before = firstCount[lhsIdx];
                addToSet(FIRST[lhsIdx], &firstCount[lhsIdx], "epsilon");
                if (firstCount[lhsIdx] > before) changed = 1;
            }
            else {
                int addEpsilon = 1;
                for (int j = 0; j < productions[i].rhsCount && addEpsilon; j++) {
                    char *sym = productions[i].rhs[j];
                    if (isTerminal(sym)) {
                        int before = firstCount[lhsIdx];
                        addToSet(FIRST[lhsIdx], &firstCount[lhsIdx], sym);
                        if (firstCount[lhsIdx] > before) changed = 1;
                        addEpsilon = 0;
                    }
                    else if (isNonTerminal(sym)) {
                        int symIdx = indexOfNT(sym);
                        for (int k = 0; k < firstCount[symIdx]; k++) {
                            if (strcmp(FIRST[symIdx][k], "epsilon") != 0) {
                                int before = firstCount[lhsIdx];
                                addToSet(FIRST[lhsIdx], &firstCount[lhsIdx], FIRST[symIdx][k]);
                                if (firstCount[lhsIdx] > before) changed = 1;
                            }
                        }
                        if (containsEpsilon(FIRST[symIdx], firstCount[symIdx])) {
                            addEpsilon = 1;
                        } else {
                            addEpsilon = 0;
                        }
                    }
                }
                if (addEpsilon) {
                    int before = firstCount[lhsIdx];
                    addToSet(FIRST[lhsIdx], &firstCount[lhsIdx], "epsilon");
                    if (firstCount[lhsIdx] > before) changed = 1;
                }
            }
        }
    } while (changed);
}
void computeFollow() {
    for (int i = 0; i < nNonTerminals; i++) followCount[i] = 0;

    addToSet(FOLLOW[0], &followCount[0], "$");

    int changed;
    do {
        changed = 0;
        for (int i = 0; i < nProductions; i++) {
            int lhsIdx = indexOfNT(productions[i].lhs);
            for (int j = 0; j < productions[i].rhsCount; j++) {
                char *B = productions[i].rhs[j];
                if (isNonTerminal(B)) {
                    int Bidx = indexOfNT(B);
                    int before = followCount[Bidx];
                    int k = j + 1;
                    int canHaveEpsilon = 1;
                    while (k < productions[i].rhsCount && canHaveEpsilon) {
                        char *beta = productions[i].rhs[k];
                        if (isTerminal(beta)) {
                            addToSet(FOLLOW[Bidx], &followCount[Bidx], beta);
                            canHaveEpsilon = 0;
                        }
                        else if (isNonTerminal(beta)) {
                            int betaIdx = indexOfNT(beta);
                            for (int m = 0; m < firstCount[betaIdx]; m++) {
                                if (strcmp(FIRST[betaIdx][m], "epsilon") != 0) {
                                    addToSet(FOLLOW[Bidx], &followCount[Bidx], FIRST[betaIdx][m]);
                                }
                            }
                            if (containsEpsilon(FIRST[betaIdx], firstCount[betaIdx])) {
                                canHaveEpsilon = 1;
                                k++;
                            } else {
                                canHaveEpsilon = 0;
                            }
                        }
                    }
                    if (canHaveEpsilon) {
                        addAll(FOLLOW[Bidx], &followCount[Bidx], FOLLOW[lhsIdx], followCount[lhsIdx]);
                    }
                    if (followCount[Bidx] > before) changed = 1;
                }
            }
        }
    } while (changed);
}
void addProductionToCell(int row, int col, Production *prod) {
    char prodStr[MAX_PARSE_ENTRY_LEN] = "";
    for (int k = 0; k < prod->rhsCount; k++) {
        strcat(prodStr, prod->rhs[k]);
        if (k != prod->rhsCount - 1)
            strcat(prodStr, " ");
    }
    if (strlen(parseTable[row][col]) == 0) {
        strcpy(parseTable[row][col], prodStr);
    } else {
        strcat(parseTable[row][col], " | ");
        strcat(parseTable[row][col], prodStr);
        isLL1 = 0;
    }
}
void constructParseTable() {
    for (int i = 0; i < nNonTerminals; i++)
        for (int j = 0; j < nTerminals; j++)
            strcpy(parseTable[i][j], "");

    isLL1 = 1;
    for (int p = 0; p < nProductions; p++) {
        int lhsIdx = indexOfNT(productions[p].lhs);
        if (lhsIdx == -1) continue;
        char firstRHS[MAX_FIRST_FOLLOW][MAX_SYMBOL_LEN];
        int firstRHSCount = 0;
        int canHaveEpsilon = 1;
        for (int i = 0; i < productions[p].rhsCount && canHaveEpsilon; i++) {
            char *sym = productions[p].rhs[i];
            if (isTerminal(sym)) {
                addToSet(firstRHS, &firstRHSCount, sym);
                canHaveEpsilon = 0;
            }
            else if (isNonTerminal(sym)) {
                int idxSym = indexOfNT(sym);
                for (int j = 0; j < firstCount[idxSym]; j++) {
                    if (strcmp(FIRST[idxSym][j], "epsilon") != 0)
                        addToSet(firstRHS, &firstRHSCount, FIRST[idxSym][j]);
                }
                if (containsEpsilon(FIRST[idxSym], firstCount[idxSym]))
                    canHaveEpsilon = 1;
                else
                    canHaveEpsilon = 0;
            }
            else if (strcmp(sym, "epsilon") == 0) {
                addToSet(firstRHS, &firstRHSCount, "epsilon");
                canHaveEpsilon = 0;
            }
        }
        if (canHaveEpsilon) addToSet(firstRHS, &firstRHSCount, "epsilon");
        for (int i = 0; i < firstRHSCount; i++) {
            if (strcmp(firstRHS[i], "epsilon") != 0) {
                int col = indexOfT(firstRHS[i]);
                if (col != -1) {
                    addProductionToCell(lhsIdx, col, &productions[p]);
                }
            }
        }
        if (containsEpsilon(firstRHS, firstRHSCount)) {
            for (int i = 0; i < followCount[lhsIdx]; i++) {
                int col = indexOfT(FOLLOW[lhsIdx][i]);
                if (col != -1) {
                    addProductionToCell(lhsIdx, col, &productions[p]);
                }
            }
        }
    }
}
void displayFirst() {
    printf("\nFIRST sets:\n");
    for (int i = 0; i < nNonTerminals; i++) {
        printf("FIRST(%s) = { ", nonTerminals[i]);
        for (int j = 0; j < firstCount[i]; j++) {
            printf("%s", FIRST[i][j]);
            if (j != firstCount[i] - 1) printf(", ");
        }
        printf(" }\n");
    }
}
void displayFollow() {
    printf("\nFOLLOW sets:\n");
    for (int i = 0; i < nNonTerminals; i++) {
        printf("FOLLOW(%s) = { ", nonTerminals[i]);
        for (int j = 0; j < followCount[i]; j++) {
            printf("%s", FOLLOW[i][j]);
            if (j != followCount[i] - 1) printf(", ");
        }
        printf(" }\n");
    }
}
void displayParseTable() {
    printf("\nPredictive Parse Table:\n");
    printf("%-10s", "");
    for (int i = 0; i < nTerminals; i++) {
        printf("%-25s", terminals[i]);
    }
    printf("\n");
    for (int i = 0; i < nNonTerminals; i++) {
        printf("%-10s", nonTerminals[i]);
        for (int j = 0; j < nTerminals; j++) {
            if (strlen(parseTable[i][j]) == 0)
                printf("%-25s", "-");
            else
                printf("%-25s", parseTable[i][j]);
        }
        printf("\n");
    }
    if (isLL1)
        printf("\nThe grammar IS LL(1).\n");
    else
        printf("\nThe grammar is NOT LL(1) due to multiple productions in some parse table cells.\n");
}
int main() {
    inputGrammar();
    computeFirst();
    computeFollow();
    constructParseTable();
    displayFirst();
    displayFollow();
    displayParseTable();
    return 0;
}
