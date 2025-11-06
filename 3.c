#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 20

char state_names[MAX][10];
char symbols[MAX];
int nfa[MAX][MAX][MAX];
int nfa_count[MAX][MAX];
int n_states, n_symbols, n_final;
int final_states[MAX];
int start_state;

int dfa_states[MAX][MAX];
int dfa_size[MAX];
int dfa_count = 0;
int dfa_trans[MAX][MAX];
int dfa_final[MAX];

int find_state_index(char name[]) {
    for(int i = 0; i < n_states; i++) {
        if(strcmp(state_names[i], name) == 0) return i;
    }
    return -1;
}

int find_symbol_index(char sym) {
    for(int i = 0; i < n_symbols; i++) {
        if(symbols[i] == sym) return i;
    }
    return -1;
}

int is_equal_set(int set1[], int size1, int set2[], int size2) {
    if(size1 != size2) return 0;
    for(int i = 0; i < size1; i++) {
        int found = 0;
        for(int j = 0; j < size2; j++) {
            if(set1[i] == set2[j]) {
                found = 1;
                break;
            }
        }
        if(!found) return 0;
    }
    return 1;
}

int find_dfa_state(int set[], int size) {
    for(int i = 0; i < dfa_count; i++) {
        if(is_equal_set(dfa_states[i], dfa_size[i], set, size)) {
            return i;
        }
    }
    return -1;
}

void sort_set(int set[], int size) {
    for(int i = 0; i < size-1; i++) {
        for(int j = i+1; j < size; j++) {
            if(set[i] > set[j]) {
                int temp = set[i];
                set[i] = set[j];
                set[j] = temp;
            }
        }
    }
}

int add_dfa_state(int set[], int size) {
    sort_set(set, size);
    for(int i = 0; i < size; i++) {
        dfa_states[dfa_count][i] = set[i];
    }
    dfa_size[dfa_count] = size;
    
    dfa_final[dfa_count] = 0;
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < n_final; j++) {
            if(set[i] == final_states[j]) {
                dfa_final[dfa_count] = 1;
                break;
            }
        }
    }
    
    return dfa_count++;
}

void compute_transition(int state_set[], int size, int symbol, int result[], int *result_size) {
    *result_size = 0;
    int temp[MAX];
    int temp_size = 0;
    
    for(int i = 0; i < size; i++) {
        int state = state_set[i];
        for(int j = 0; j < nfa_count[state][symbol]; j++) {
            int next = nfa[state][symbol][j];
            int exists = 0;
            for(int k = 0; k < temp_size; k++) {
                if(temp[k] == next) {
                    exists = 1;
                    break;
                }
            }
            if(!exists) {
                temp[temp_size++] = next;
            }
        }
    }
    
    for(int i = 0; i < temp_size; i++) {
        result[i] = temp[i];
    }
    *result_size = temp_size;
}

void convert_nfa_to_dfa() {
    int initial[1] = {start_state};
    add_dfa_state(initial, 1);
    
    int processed = 0;
    while(processed < dfa_count) {
        for(int sym = 0; sym < n_symbols; sym++) {
            int result[MAX], result_size;
            compute_transition(dfa_states[processed], dfa_size[processed], sym, result, &result_size);
            
            if(result_size > 0) {
                int idx = find_dfa_state(result, result_size);
                if(idx == -1) {
                    idx = add_dfa_state(result, result_size);
                }
                dfa_trans[processed][sym] = idx;
            } else {
                dfa_trans[processed][sym] = -1;
            }
        }
        processed++;
    }
}

int simulate_nfa(char *input) {
    int current_states[MAX], current_size = 1;
    current_states[0] = start_state;
    
    for(int i = 0; i < strlen(input); i++) {
        int sym_idx = find_symbol_index(input[i]);
        if(sym_idx == -1) return 0;
        
        int next_states[MAX], next_size = 0;
        for(int j = 0; j < current_size; j++) {
            int state = current_states[j];
            for(int k = 0; k < nfa_count[state][sym_idx]; k++) {
                int next = nfa[state][sym_idx][k];
                int exists = 0;
                for(int l = 0; l < next_size; l++) {
                    if(next_states[l] == next) {
                        exists = 1;
                        break;
                    }
                }
                if(!exists) {
                    next_states[next_size++] = next;
                }
            }
        }
        
        if(next_size == 0) return 0;
        
        for(int j = 0; j < next_size; j++) {
            current_states[j] = next_states[j];
        }
        current_size = next_size;
    }
    
    for(int i = 0; i < current_size; i++) {
        for(int j = 0; j < n_final; j++) {
            if(current_states[i] == final_states[j]) {
                return 1;
            }
        }
    }
    return 0;
}

int simulate_dfa(char *input) {
    int current = 0;
    
    for(int i = 0; i < strlen(input); i++) {
        int sym_idx = find_symbol_index(input[i]);
        if(sym_idx == -1) return 0;
        
        int next = dfa_trans[current][sym_idx];
        if(next == -1) return 0;
        current = next;
    }
    
    return dfa_final[current];
}

int main() {
    memset(nfa_count, 0, sizeof(nfa_count));
    memset(dfa_trans, -1, sizeof(dfa_trans));
    
    printf("Enter number of NFA states: ");
    scanf("%d", &n_states);
    
    printf("Enter state names:\n");
    for(int i = 0; i < n_states; i++) {
        scanf("%s", state_names[i]);
    }
    
    printf("Enter number of input symbols: ");
    scanf("%d", &n_symbols);
    
    printf("Enter input symbols:\n");
    for(int i = 0; i < n_symbols; i++) {
        scanf(" %c", &symbols[i]);
    }
    
    printf("Enter transitions for NFA:\n");
    printf("Enter -1 to stop.\n");
    
    char from[10], to[10], sym;
    while(1) {
        scanf("%s", from);
        if(strcmp(from, "-1") == 0) break;
        
        scanf(" %c %s", &sym, to);
        
        int from_idx = find_state_index(from);
        int to_idx = find_state_index(to);
        int sym_idx = find_symbol_index(sym);
        
        if(from_idx == -1 || to_idx == -1 || sym_idx == -1) {
            printf("Invalid transition!\n");
            continue;
        }
        
        nfa[from_idx][sym_idx][nfa_count[from_idx][sym_idx]++] = to_idx;
    }
    
    printf("Enter number of NFA final states: ");
    scanf("%d", &n_final);
    
    printf("Enter NFA accepting state names:\n");
    for(int i = 0; i < n_final; i++) {
        char fname[10];
        scanf("%s", fname);
        final_states[i] = find_state_index(fname);
    }
    
    start_state = 0;
    
    convert_nfa_to_dfa();
    
    printf("\nDFA Transition Table:\n");
    printf("State\t");
    for(int i = 0; i < n_symbols; i++) {
        printf("%c\t", symbols[i]);
    }
    printf("\n");
    
    for(int i = 0; i < dfa_count; i++) {
        for(int j = 0; j < dfa_size[i]; j++) {
            printf("%s", state_names[dfa_states[i][j]]);
            if(j < dfa_size[i]-1) printf(",");
        }
        printf("\t");
        
        for(int j = 0; j < n_symbols; j++) {
            int next = dfa_trans[i][j];
            if(next != -1) {
                for(int k = 0; k < dfa_size[next]; k++) {
                    printf("%s", state_names[dfa_states[next][k]]);
                    if(k < dfa_size[next]-1) printf(",");
                }
            } else {
                printf("-");
            }
            printf("\t");
        }
        printf("\n");
    }
    
    printf("\nDFA Accepting States:\n");
    for(int i = 0; i < dfa_count; i++) {
        if(dfa_final[i]) {
            for(int j = 0; j < dfa_size[i]; j++) {
                printf("%s", state_names[dfa_states[i][j]]);
                if(j < dfa_size[i]-1) printf(",");
            }
            printf("\n");
        }
    }
    
    char input[100];
    printf("\nEnter string to check: ");
    scanf("%s", input);
    
    printf("Result for input string \"%s\":\n", input);
    printf("NFA: %s\n", simulate_nfa(input) ? "Accepted" : "Rejected");
    printf("DFA: %s\n", simulate_dfa(input) ? "Accepted" : "Rejected");
    
    return 0;
}