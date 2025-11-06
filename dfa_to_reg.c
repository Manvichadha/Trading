#include<stdio.h>
#include<string.h>
#define STATE 50
#define NAME 32
#define INPUT 32
#define T 1000
int findex(char arr[][NAME], int n, const char *name)
{
    for (int i = 0; i < n; i++)
    {
        if (strcmp(arr[i], name) == 0)
            return i;
    }
    return -1;
}
int main()
{
    int n;//number of states
    printf("Enter number of states: ");
    if (scanf("%d", &n) != 1) return 0;
    char states[STATE][NAME];
    printf("Enter states: ");
    for (int i = 0; i < n; i++)
    {
        scanf("%s", states[i]);
    }
    char start[NAME];
    printf("Enter start state: ");
    scanf("%s", start);
    int si = findex(states, n, start);
    if (si == -1) si = 0;
    int m; //number of inputs
    printf("Enter number of input symbols: ");
    scanf("%d", &m);
    char inputs[INPUT][NAME];
    printf("Enter input symbols: ");
    for (int i = 0; i < m; i++)
    {
        scanf("%s", inputs[i]);
    }
    int final[STATE] = {0};
    int count; //number of final states
    printf("Enter number of final states: ");
    scanf("%d", &count);
    if(count > 0)
    {
        printf("Enter final states: ");
        for (int i = 0; i < count; i++)
        {
            char fname[NAME];
            scanf("%s", fname);
            int fi = findex(states, n, fname);
            if (fi != -1)
            {
                final[fi] = 1;
            }
        }
        
    }
    else
    {
    }
    int trans[STATE][INPUT];
    for(int i =0; i<n; i++)
    {
        for(int j =0; j<m; j++)
        {           
            trans[i][j] = -1;
        }
    }
    int t;
    printf("\nEnter number of transitions: ");
    scanf("%d", &t);
    printf("Enter transitions (format: from symbol to) :\n");
    for(int i=0; i< t; i++)
    {
        char from[NAME], to[NAME], symbol[NAME];
        scanf("%s %s %s", from, symbol, to);
        int fi = findex(states, n, from);
        int ti = findex(states, n, to);
        si = -1;
        for(int j =0; j<m; j++)
        {
            if(strcmp(inputs[j], symbol) == 0)
            {
                si = j;
                break;
            }
        }
        if(fi != -1 && ti != -1 && si != -1)
        {
            trans[fi][si] = ti;
        }
    }
    printf("\n\nEnter transitions:\n");
    for (int i = 0; i < n; ++i) 
    {
        for (int j = 0; j < m; ++j)
        {
            printf("D[%s, %d] = ", states[i], j);
            if (trans[i][j] >= 0) 
            printf("%s\n", states[ trans[i][j] ]);
            else 
            printf("-\n");
        }
    }
    printf("\nRegular Grammar G = (N, T, P, S)\n");
    /* N */
    printf("N = { ");
    for (int i = 0; i < n; ++i) {
        printf("%s", states[i]);
        if (i < n-1) 
        printf(" ");
    }
    printf(" }\n");

    /* T */
    printf("T = { ");
    for (int i = 0; i < m; ++i) {
        printf("%s", inputs[i]);
        if (i < m-1) 
        printf(" ");
    }
    printf(" }\n");

    /* S */
    printf("S = %s\n", states[si]);

    /* P */
    printf("P = {\n");
    for (int i = 0; i < n; ++i) {
        int pri = 0;
        for (int j = 0; j < m; ++j) {
            int k = trans[i][j];
            if (k >= 0) {
                printf("  %s -> %s %s\n", states[i], inputs[j], states[k]);
                pri = 1;
                if (final[k]) {
                    printf("  %s -> %s\n", states[i], inputs[j]);
                }
            }
        }
        if (final[i]) {
            printf("  %s -> ε\n", states[i]);
            pri = 1;
        }
        if (!pri) {
            printf("  %s -> (no productions)\n", states[i]);
        }
    }
    printf("}\n");

    return 0;
}
