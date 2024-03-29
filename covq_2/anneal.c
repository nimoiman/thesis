#include "covq.h"

// Simulated Annealing Constants
#define TEMP_INIT 10.0
#define COOLING_RATE 0.8
#define TEMP_FINAL 0.00025
#define PHI 5 // energy drops until temperature drop
#define PSI 200 // rejected swaps until temperature drop

/* Helper function to swap integers *i and *j */
void swap(int *i, int *j) {
    int tmp = *i;
    *i = *j;
    *j = tmp;
}

/* get energy of current binary index assignment */
double energy(int codebook_count[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE],
        covq2 *v) {
    double sum = 0;

    for (int i = 0; i < v->N_X; i++) {
        for (int j = 0; j < v->N_Y; j++) {
            double inner_sum = 0;
            for (int k = 0; k < v->N_X; k++) {
                for (int l = 0; l < v->N_Y; l++) {
                    double p = v->trans_prob(i, j, k, l, v->b_X, v->b_Y);
                    inner_sum += (POW2(v->x_ij[i][j] - v->x_ij[k][l]) +
                                POW2(v->y_ij[i][j] - v->y_ij[k][l])) * p;
                }
            }
            sum += inner_sum * codebook_count[i][j];
        }
    }

    return sum / v->q->npoints;
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

void anneal(covq2 *v) {
    int drop_count = 0, fail_count = 0;
    int codebook_count[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];

    /*
     * Compute codebook_count
     * Should contain number of training points that map to each transmission
     * pair (i,j).
     */
    for (int i = 0; i < v->N_X; i++) {
        for (int j = 0; j < v->N_Y; j++) {
            codebook_count[i][j] = 0;
        }
    }

    for (int qx = 0; qx < v->q->L_X; qx++) {
        int i = v->I_X[qx];
        for (int qy = 0; qy < v->q->L_Y; qy++) {
            int j = v->I_Y[qy];
            codebook_count[i][j] += quantizer_get_count(qx, qy, v->q);
        }
    }

    /*
     * Get initial energy of system. Save as E_old.
     */
    double E_old = energy(codebook_count, v);

    /*
     * Begin annealing process.
     * Loop until we reach final temperature.
     */
    double T = TEMP_INIT;
    while (T > TEMP_FINAL) {

        /*
         * Swap two random indexes for both sources. Swap i1 with i2 and j1
         * with j2.
         */
        int i1 = rand_lim(v->N_X);
        int i2 = rand_lim(v->N_X);
        swap(v->b_X + i1, v->b_X + i2);
        int j1 = rand_lim(v->N_Y);
        int j2 = rand_lim(v->N_Y);
        swap(v->b_Y + j1, v->b_Y + j2);

        /*
         * Compute energy of new system.
         */
        double E_new = energy(codebook_count, v);
        
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
            swap(v->b_X + i1, v->b_X + i2);
            swap(v->b_Y + j1, v->b_Y + j2);
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
