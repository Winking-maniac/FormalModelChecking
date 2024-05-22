#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

typedef struct Clause {
    size_t size;
    int16_t *lits;
} Clause;

typedef char *Tetrits;

typedef enum State {
    TRUE = 1,
    FALSE = 2,
    UNSET = 0
} State;

size_t N_VARS;
size_t N_CLAUSES;
Clause *CLAUSES;

void
tetrits_copy(Tetrits src, Tetrits dst) {
    memcpy(dst - N_VARS, src - N_VARS, 2* N_VARS + 1);
}

void
set(Tetrits t, int16_t ind) {
    t[ind] = TRUE;
    t[-ind] = FALSE;
}

State
get(Tetrits t, int16_t ind) {
    return t[ind];
}

typedef struct Frame {
    Tetrits inter;
} Frame;   

typedef enum SolverRes {
    SAT,
    UNSAT,
    UNKNOWN
} SolverRes;

Clause
clause_init() {
    Clause res;
    res.size = 0;
    res.lits = calloc(N_VARS, sizeof(*res.lits));
    int16_t cur;
    (void)! scanf("%"SCNd16, &cur);
    while (cur != 0) {
        res.lits[res.size] = cur;
        res.size++;
        (void)! scanf("%"SCNd16, &cur);
    }
    res.lits = realloc(res.lits, res.size * sizeof(*res.lits));
    assert(res.lits);
    return res;
}

// Desc: make step - propagate param of cur frame and all that follows,
// if SAT returns SAT, else chooses next suggestion, fills stack and returns UNKNOWN
SolverRes
solve(Frame *stack, size_t *cur_frame);

// Desc: propagate param
// Returns SAT, UNSAT or UNKNOWN
SolverRes
prop_one(Frame *fr);

// Desc: choose param for split using heuristics
// Returns new param
int16_t
calc_param(Frame fr);

int
main(int argc, char **argv) {
    char c;
    (void)! scanf("%c", &c);
    while (c == 'c') {
        if (scanf("%*[^\n]%c", &c) == 0) {
            (void)! scanf("%c", &c);
        }
        (void)! scanf("%c", &c);
    }
    (void)! scanf(" cnf %lu %lu", &N_VARS, &N_CLAUSES);

    Frame *stack = calloc(N_VARS + 1, sizeof(*stack));
    stack[0].inter = calloc((N_VARS + 1) * (2 * N_VARS + 1), sizeof(*stack[0].inter)) + N_VARS;
    for (size_t i = 1; i < N_VARS + 1; ++i) {
        stack[i].inter = stack[i - 1].inter + 2 * N_VARS + 1;
    }
    size_t stack_size = 1;
    CLAUSES = calloc(N_CLAUSES, sizeof(*CLAUSES));

    size_t clauses_size = 0;
    for (size_t i = 0; i < N_CLAUSES; ++i) {
        CLAUSES[i] = clause_init();
        clauses_size += CLAUSES[i].size;
    }
    for (size_t i = 0; i < N_CLAUSES; ++i) {
        for (size_t j = i + 1; j < N_CLAUSES; ++j) {
            size_t max_left = 0;
            for (size_t k = 0; k < CLAUSES[j - 1].size; ++k) {
                if (CLAUSES[j - 1].lits[k] > 0) {
                    max_left = max_left > CLAUSES[j - 1].lits[k] ? max_left : CLAUSES[j - 1].lits[k];
                } else {
                    max_left = max_left > -CLAUSES[j - 1].lits[k] ? max_left : -CLAUSES[j - 1].lits[k];
                }
            }
            size_t max_right = 0;
            for (size_t k = 0; k < CLAUSES[j].size; ++k) {
                if (CLAUSES[j].lits[k] > 0) {
                    max_right = max_right > CLAUSES[j].lits[k] ? max_right : CLAUSES[j].lits[k];
                } else {
                    max_right = max_right > -CLAUSES[j].lits[k] ? max_right : -CLAUSES[j].lits[k];
                }
            }
            if (max_right < max_left || (max_right == max_left && CLAUSES[j - 1].size > CLAUSES[j].size)) {
                Clause tmp = CLAUSES[j - 1];
                CLAUSES[j - 1] = CLAUSES[j];
                CLAUSES[j] = tmp;
            }
        }
    }
    int16_t *clause_arr = calloc(clauses_size, sizeof(*clause_arr));
    size_t cnt = 0;
    for (size_t i = 0; i < N_CLAUSES; ++i) {
        size_t start = cnt;
        for (size_t j = 0; j < CLAUSES[i].size; ++j, ++cnt) {
            clause_arr[cnt] = CLAUSES[i].lits[j];
        }
        free(CLAUSES[i].lits);
        CLAUSES[i].lits = clause_arr + start;
    }
    // Main loop
    while (stack_size > 0) {
        SolverRes solver_result = solve(stack, &stack_size);
        if (solver_result == SAT) {
            printf("SAT\n");
            free(stack[0].inter - N_VARS);
            free(stack);
            free(CLAUSES[0].lits);
            free(CLAUSES);
            return 0;
        } else if (solver_result == UNSAT) {
            fprintf(stderr, "Unexpected UNSAT in main loop");
            abort();
        }
    }
    // Truly UNSAT
    printf("UNSAT\n");
    free(stack[0].inter - N_VARS);
    free(stack);
    free(CLAUSES[0].lits);
    free(CLAUSES);
    return 0;
} 

SolverRes
solve(Frame *stack, size_t *stack_size) {
    size_t cf = *stack_size - 1;
    SolverRes res = prop_one(stack + cf);
    if (res == SAT) {
        for (size_t i = 1; i <= N_VARS; ++i) {
            State s = get(stack[cf].inter, i); 
            printf("%lu: %s\n", i, s == TRUE ? "True" : (s == FALSE ? "False" : "Unset"));
        }
        return SAT;
    } else if (res == UNSAT) {
        *stack_size = cf;
        return UNKNOWN;
    }
    int16_t new_param = calc_param(stack[cf]);
    *stack_size = cf + 2;
    tetrits_copy(stack[cf].inter, stack[cf + 1].inter);
    set(stack[cf].inter, -new_param);
    set(stack[cf + 1].inter, new_param);
    return UNKNOWN;
}

SolverRes
prop_one(Frame *fr) {
    char not_done = 1;
    while (not_done) {
        char sat = 1;
        not_done = 0;
        for (size_t i = 0; i < N_CLAUSES; ++i) {
            size_t unk_cnt = 0;
            int16_t unk = 0;
            for (size_t j = 0; j < CLAUSES[i].size; ++j) {
                switch (get(fr->inter, CLAUSES[i].lits[j])) {
                    case TRUE:
                        goto NEXT_CLAUSE;
                    case UNSET:
                        unk_cnt++;
                        unk = CLAUSES[i].lits[j];
                    case FALSE:
                        ;
                }
            }
            if (unk_cnt == 0) {
                return UNSAT;
            } else if (unk_cnt == 1) {
                set(fr->inter, unk);
                not_done = 1;
            } else {
                sat = 0;
            }
NEXT_CLAUSE:;
        }
        if (sat) {
            return SAT;
        }
    }
    return UNKNOWN;
}

int16_t
calc_param(Frame fr) {
    // Dummy
    for (size_t i = 1; i <= N_VARS; ++i) {
        if (get(fr.inter, i) == UNSET) {
            return i;
        }
    }
    assert(0);
}
