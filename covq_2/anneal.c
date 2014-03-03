#include "covq.h"

// Simulated Annealing Constants
#define TEMP_INIT 10.0
#define COOLING_RATE 0.8
#define TEMP_FINAL 0.00025
#define PHI 5 // energy drops until temperature drop
#define PSI 200 // rejected swaps until temperature drop

/* Helper function to swap integers *i and *j */
void swap(int *i, int *j) {
    int tmp;
    tmp = *i;
    *i = *j;
    *j = tmp;
}

/* get energy of current binary index assignment */
double energy(int codebook_count[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE],
        params_covq2 *p, covq2 *c) {
    int i, j, k, el;
    double eucl_dist;
    double sum = 0;
    double inner_sum;

    for (i = 0; i < CODEBOOK_SIZE_X; i++) {
        for (j = 0; j < CODEBOOK_SIZE_Y; j++) {
            inner_sum = 0;
            for (k = 0; k < CODEBOOK_SIZE_X; k++) {
                for (el = 0; el < CODEBOOK_SIZE_Y; el++) {
                    eucl_dist = POW2(c->codevec_x[CV_IDX(i,j)] - c->codevec_x[CV_IDX(k,el)]) +
                                POW2(c->codevec_y[CV_IDX(i,j)] - c->codevec_y[CV_IDX(k, el)]);
                    inner_sum += p->transition_prob(i, j, k, el, p, c) * eucl_dist;
                }
            }
            assert(inner_sum >= 0);
            sum += inner_sum * codebook_count[i][j];
        }
    }
    return sum / p->trset_size;
}

/* return a random number between 0 and limit-1 inclusive.
 */
int rand_lim(int limit) {
    int divisor = RAND_MAX/(limit);
    int retval;

    do { 
        retval = rand() / divisor;
    } while (retval >= limit);

    return retval;
}

void anneal( params_covq2 *p, covq2 *c ) {
    double E_new, E_old;
    double T = TEMP_INIT;
    int drop_count = 0, fail_count = 0;
    int codebook_count[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];
    int i1, j1, i2, j2;
    int i,j;
    int q_x, q_y;

    /*
     * Compute codebook_count
     * Should contain number of training points that map to each transmission
     * pair (i,j).
     */
    for (i = 0; i < CODEBOOK_SIZE_X; i++) {
        for (j = 0; j < CODEBOOK_SIZE_Y; j++) {
            codebook_count[i][j] = 0;
        }
    }

    for (q_x = 0; q_x < p->qlvls; q_x++) {
        i = c->encoder_x[q_x];
        for (q_y = 0; q_y < p->qlvls; q_y++) {
            j = c->encoder_y[q_y];
            codebook_count[i][j] += c->qtrset[TS_IDX(i,j)];
        }
    }

    /*
     * Get initial energy of system. Save as E_old.
     */
    E_old = energy(codebook_count, p, c);

    /*
     * Begin annealing process.
     * Loop until we reach final temperature.
     */
    while (T > TEMP_FINAL) {

        /*
         * Swap two random indexes for both sources. Swap i1 with i2 and j1
         * with j2.
         */
        i1 = rand_lim(CODEBOOK_SIZE_X);
        i2 = rand_lim(CODEBOOK_SIZE_X);
        swap(c->cwmap_x + i1, c->cwmap_x + i2);
        j1 = rand_lim(CODEBOOK_SIZE_Y);
        j2 = rand_lim(CODEBOOK_SIZE_Y);
        swap(c->cwmap_y + j1, c->cwmap_y + j2);

        /*
         * Compute energy of new system.
         */
        E_new = energy(codebook_count, p, c);
        
        /*
         * If new state is better than old state, keep it. Otherwise keep the
         * new state with probability exp(-(E_new-E_old)/T).
         */
        if (E_new <= E_old) {
            E_old = E_new;
            drop_count++;
        }
        else if ((rand() / (double) RAND_MAX) < exp(-(E_new-E_old)/T)) {
            E_old = E_new;
            fail_count++;
        }
        else {
            fail_count++;
            // Swap back to old system
            swap(c->cwmap_x + i1, c->cwmap_x + i2);
            swap(c->cwmap_y + j1, c->cwmap_y + j2);
        }

        /*
         * After enough drops or fails we lower the temerature
         */
        if (drop_count == PHI || fail_count == PSI) {
            T *= COOLING_RATE;
            drop_count = fail_count = 0;
        }
    }
}
