#include<stdio.h>
#include<string.h>
#define STATE 50
#define NAME 32
#define INPUT 32
#define STR 1000
int main()
{
    int n,m;
    printf("Enter number of states: ");
    if(scanf("%d" ,&n)!=1) return 0;
    char states[STATE][NAME];
    printf("Enter states: ");
    for(int i =0; i<n; i++)
    {
        scanf("%s", states[i]);
    }
    printf("Enter number of input symbols: ");
    if (scanf("%d", &m) != 1) return 0;
    char alphabets[INPUT];
    printf("Enter input symbols: ");
    for(int i =0; i<m; i++)
    {
        scanf(" %c", &alphabets[i]);
    }
    int final[STATE] ={0};
    int count;
    printf("Enter number of final states: ");
    scanf("%d", &count);
    printf("Enter final states: ");
    for(int i = 0; i<count; i++)
    {
        char fname[NAME];
        scanf("%s", fname);
        for(int j =0; j<n; j++)
        {
            if(strcmp(fname, states[j]) == 0)
            {
                final[j] = 1;
                break;
            }
        }
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
        char from[NAME], to[NAME];
        char symbol;
        scanf("%s %c %s", from, &symbol, to);
        int fi = -1, ti = -1, si = -1;
        for(int j =0; j<n; j++)
        {           
            if(strcmp(from, states[j]) == 0)
            {
                fi = j;
            }
        }
        for(int j =0; j<n; j++)
        {
            if(strcmp(to, states[j]) == 0)
            {
                ti = j;
            }
        }
        for(int j =0; j<n; j++)
        {
            if(symbol == alphabets[j])
            {
                si = j;
            }
        }
        if(fi != -1 && ti != -1 && si != -1)
        {
            trans[fi][si] = ti;
        }
    }
    char w[STR+1];
    printf("\nEnter input string: ");
    scanf("%s", w);
    printf("\nTransition table (rows=state, cols=symbol):\n     ");
    for(int j =0; j<m; j++)
    {   
        printf("\t%c\t ", alphabets[j]);
    }
    printf("\n");
    for(int k =0; k<n; k++)
    {
        printf("%-6s%s ",states[k], final[k] ? "(F)" : "    ");
        for(int l =0; l<m; l++)
        {
            if(trans[k][l] >= 0)
            {
                printf("%-4s", states[trans[k][l]]);
            }
            else
            {
                printf(" -  ");
            }
        }
        printf("\n");  
    }
            int current = 0;
            printf("\nSimulating input \"%s\" starting at %s\n", w, states[current]);
            printf("Sequence of transitions:\n");
            for(int j =0; j<strlen(w); j++)
            {
                char ch = w[j];
                int si = -1;
                for(int k =0; k<m; k++)
                {
                    if(ch == alphabets[k])
                    {
                        si = k;
                        break;
                    }
                }
                if(si == -1)
                {
                    printf(" symbol '%c' not in alphabet -> REJECTED\n", ch);
                    return 0;
                }
                int next = trans[current][si];
                if(next == -1)
                {
                    printf(" No transition from state '%s' on symbol '%c' -> REJECTED\n", states[current], ch);
                    return 0;
                }
                printf(" %s --%c--> %s\n", states[current], ch, states[next]);
                current = next;
            }
            if(final[current])
            {
                printf(" String accepted, ended in final state '%s'\n", states[current]);
            }
            else
            {
                printf(" String rejected, ended in non-final state '%s'\n", states[current]);
            }
    return 0;
}
