#include <stdio.h>
#include <string.h>

#define MAX_NFA 16      // max NFA states (change if you understand exponential DFA blowup)
#define MAX_INPUT 16    // max input symbols
#define NAME 32
#define MAX_DFA (1 << MAX_NFA)

int findex(char arr[][NAME], int n, const char *name) {
    for (int i = 0; i < n; ++i) if (strcmp(arr[i], name) == 0) return i;
    return -1;
}

/* print mask as set of state names, or ∅ if empty */
void print_mask(unsigned int mask, char states[][NAME], int n) {
    if (mask == 0) { printf("∅"); return; }
    int first = 1;
    printf("{");
    for (int i = 0; i < n; ++i) {
        if (mask & (1u << i)) {
            if (!first) printf(",");
            printf("%s", states[i]);
            first = 0;
        }
    }
    printf("}");
}

int main() {
    int n;
    printf("Enter number of NFA states (<= %d): ", MAX_NFA);
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_NFA) return 0;

    char states[MAX_NFA][NAME];
    printf("Enter state names (space separated):\n");
    for (int i = 0; i < n; ++i) scanf("%s", states[i]);

    char start[NAME];
    printf("Enter start state: ");
    scanf("%s", start);
    int start_index = findex(states, n, start);
    if (start_index == -1) { printf("Start state not found.\n"); return 0; }

    int m;
    printf("Enter number of input symbols (<= %d): ", MAX_INPUT);
    scanf("%d", &m);
    if (m <= 0 || m > MAX_INPUT) return 0;
    char inputs[MAX_INPUT][NAME];
    printf("Enter input symbols (space separated):\n");
    for (int i = 0; i < m; ++i) scanf("%s", inputs[i]);

    int final_count;
    printf("Enter number of final states: ");
    scanf("%d", &final_count);
    unsigned int final_mask = 0;
    if (final_count > 0) {
        printf("Enter final state names (space separated):\n");
        for (int i = 0; i < final_count; ++i) {
            char t[NAME]; scanf("%s", t);
            int fi = findex(states, n, t);
            if (fi != -1) final_mask |= (1u << fi);
        }
    }

    /* NFA transitions: for each state and input symbol, store bitmask of destinations */
    unsigned int trans_nfa[MAX_NFA][MAX_INPUT];
    for (int i = 0; i < MAX_NFA; ++i) for (int j = 0; j < MAX_INPUT; ++j) trans_nfa[i][j] = 0;

    int t;
    printf("Enter number of transitions: ");
    scanf("%d", &t);
    printf("Enter transitions (format: from symbol to) one per line, e.g. q0 a q1\n");
    for (int i = 0; i < t; ++i) {
        char from[NAME], sym[NAME], to[NAME];
        scanf("%s %s %s", from, sym, to);
        int fi = findex(states, n, from);
        int ti = findex(states, n, to);
        int si = findex(inputs, m, sym);
        if (fi == -1 || ti == -1 || si == -1) {
            printf("Ignoring invalid transition: %s %s %s\n", from, sym, to);
            continue;
        }
        trans_nfa[fi][si] |= (1u << ti); // add destination
    }

    /* Subset construction */
    int map[MAX_DFA];            // map mask -> dfa index (-1 if unseen)
    for (int i = 0; i < (1 << n); ++i) map[i] = -1;

    unsigned int queue[MAX_DFA];
    unsigned int dfa_masks[MAX_DFA];
    unsigned int dfa_trans[MAX_DFA][MAX_INPUT];
    int dfa_final[MAX_DFA];
    int qh = 0, qt = 0;
    int dfa_count = 0;

    unsigned int start_mask = (1u << start_index); // NFA has single start state (no eps)
    map[start_mask] = dfa_count;
    dfa_masks[dfa_count] = start_mask;
    dfa_count++;
    queue[qt++] = start_mask;

    while (qh < qt) {
        unsigned int cur_mask = queue[qh++];
        int cur_idx = map[cur_mask];
        // for each input symbol, compute destination mask
        for (int sym = 0; sym < m; ++sym) {
            unsigned int dest = 0;
            for (int s = 0; s < n; ++s) {
                if (cur_mask & (1u << s)) {
                    dest |= trans_nfa[s][sym];
                }
            }
            dfa_trans[cur_idx][sym] = dest;
            if (map[dest] == -1) {
                map[dest] = dfa_count;
                dfa_masks[dfa_count] = dest;
                dfa_count++;
                queue[qt++] = dest;
            }
        }
    }

    /* mark DFA final states */
    for (int i = 0; i < dfa_count; ++i) {
        dfa_final[i] = ( (dfa_masks[i] & final_mask) != 0 );
    }

    /* Print result */
    printf("\nDFA states (as sets of NFA states):\n");
    for (int i = 0; i < dfa_count; ++i) {
        printf("D%d = ", i);
        print_mask(dfa_masks[i], states, n);
        if (i == map[start_mask]) printf("  (start)");
        if (dfa_final[i]) printf("  (final)");
        printf("\n");
    }

    printf("\nDFA transition table:\n");
    // header
    printf("%-6s", "State");
    for (int j = 0; j < m; ++j) printf("%-15s", inputs[j]);
    printf("\n");
    for (int i = 0; i < dfa_count; ++i) {
        printf("D%-5d", i);
        for (int j = 0; j < m; ++j) {
            unsigned int dest = dfa_trans[i][j];
            int dest_idx = map[dest];
            if (dest_idx >= 0) {
                // print as Dk (set)
                char buf[64];
                sprintf(buf, "D%d", dest_idx);
                printf("%-15s", buf);
            } else {
                printf("%-15s", "—");
            }
        }
        printf("\n");
    }

    printf("\nStart DFA state: D%d\n", map[start_mask]);
    printf("Final DFA states: ");
    int any = 0;
    for (int i = 0; i < dfa_count; ++i) if (dfa_final[i]) {
        if (any) printf(", ");
        printf("D%d", i);
        any = 1;
    }
    if (!any) printf("none");
    printf("\n");

    /* also print each DFA state's set explicitly for clarity */
    printf("\nMapping of DFA states to NFA-sets:\n");
    for (int i = 0; i < dfa_count; ++i) {
        printf("D%d = ", i);
        print_mask(dfa_masks[i], states, n);
        printf("\n");
    }

    return 0;
}
