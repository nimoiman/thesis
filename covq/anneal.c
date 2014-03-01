#include "covq.h"

/* Helper function to swap integers *i and *j */
void swap(int *i, int *j) {
    int tmp;
    tmp = *i;
    *i = *j;
    *j = tmp;
}

/* get energy of current binary index assignment */
double energy(int codebook_count[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y]) {
    int i, j, k, el;
    double eucl_dist;
    double sum = 0;
    double inner_sum;
    for (i = 0; i < CODEBOOK_SIZE_X; i++) {
        for (j = 0; j < CODEBOOK_SIZE_Y; j++) {
            inner_sum = 0;
            for (k = 0; k < CODEBOOK_SIZE_X; k++) {
                for (el = 0; el < CODEBOOK_SIZE_Y; el++) {
                    eucl_dist = POW2(cv_x[i][j] - cv_x[k][el]) +
                                POW2(cv_y[i][j] - cv_y[k][el]);
                    inner_sum += channel_prob(i, j, k, el) * eucl_dist;
                }
            }
            assert(inner_sum >= 0);
            sum += inner_sum * codebook_count[i][j];
        }
    }
    return sum / trset_size;
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

void anneal() {
    double new_energy, old_energy;
    double T = TEMP_INIT;
    int drop_count = 0, fail_count = 0;
    int codebook_count[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
    int i_1, j_1, i_2, j_2;

    transmission_prob(codebook_count);
    old_energy = energy(codebook_count);

    while (T > TEMP_FINAL) {
        // randomly choose two indices in [0, CODEBOOK_SIZE_X-1]
        i_1 = rand_lim(CODEBOOK_SIZE_X);
        i_2 = rand_lim(CODEBOOK_SIZE_X);
        // randomly choose two indices in [0, CODEBOOK_SIZE_Y-1]
        j_1 = rand_lim(CODEBOOK_SIZE_Y);
        j_2 = rand_lim(CODEBOOK_SIZE_Y);

        // swap i_1 <-> i_2 & j_1 <-> j_2 temporarily
        swap(bin_cw_x + i_1, bin_cw_x + i_2);
        swap(bin_cw_y + j_1, bin_cw_y + j_2);

        // find the difference in new state's energy from old energy
        new_energy = energy(codebook_count);
        
        // keep swap if energy drop
        // else keep swap with probability e^{-rise/T}
        if (new_energy <= old_energy) {
            old_energy = new_energy;
            drop_count++;
        }
        else if ((rand() / (double) RAND_MAX) < exp(-(new_energy-old_energy)/T)) {
            old_energy = new_energy;
            fail_count++;
        }
        else {
            fail_count++;
            // don't keep swap i.e. swap back
            swap(bin_cw_x + i_1, bin_cw_x + i_2);
            swap(bin_cw_y + j_1, bin_cw_y + j_2);
        }

        if (drop_count == PHI || fail_count == PSI) {
            // lower the temperature
            T *= COOLING_RATE;
            // reset counts
            drop_count = 0;
            fail_count = 0;
        }
    }
}
