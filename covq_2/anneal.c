#include "covq.h"
#define POW2(x) (x*x)

/* Calculate the (unnormalized) probabilities of transmitting the pairs (i,j) */
void transmission_prob(prob_ij p_ij) {
    int i, j, q_x, q_y;

    // loop through quantization levels q_trset
    for (q_x = 0; q_x < Q_LEVELS; q_x++) {
        i = encoder_x[q_x];
        for (q_y = 0; q_y < Q_LEVELS; q_y++) {
            // lookup which index is transmitted for that level in enc_x, enc_y
            j = encoder_y[q_y];
            // add bin count to p_ij[i][j]
            p_ij[i][j] += q_trset[q_x][q_y];
        }
    }
}

/* Helper function to simulated annealing
 * Return distance between codevectors (centroids) represented by the two
 * respective pairs (i, j), (k, el) */
double eucl_dist(int i, int j, int k, int el) {
    return POW2(cv_x[i][j] - cv_x[k][el]) + POW2(cv_y[i][j] - cv_y[k][el]);
}

/* get energy of current binary index assignment */
double energy() {
    int i, j, k, el;
    double sum = 0;
    double inner_sum;
    prob_ij p_ij;
    transmission_prob(p_ij);
    for (i = 0; i < CODEBOOK_SIZE_X; i++) {
        for (j = 0; j < CODEBOOK_SIZE_Y; j++) {
            inner_sum = 0;
            for (k = 0; k < CODEBOOK_SIZE_X; k++) {
                for (el = 0; el < CODEBOOK_SIZE_Y; el++) {
                    inner_sum += channel_prob(i, j, k, el) * eucl_dist(i, j, k, el);
                }
            }
            sum += inner_sum * p_ij[i][j] / trset_size;
        }
    }
    return sum;
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
    double pot_diff, tmp;
    double T = 10.0, cooling_rate = 0.8, T_final = 0.025;
    int iter = 0, max_iterations = 50000;
    int phi = 5, drop_count = 0, psi = 200, fail_count = 0;
    
    int i_1, j_1, i_2, j_2;
    do {
        // randomly choose two indices between 0 and FINAL_C_SIZE_X-1
        i_1 = rand_lim(CODEBOOK_SIZE_X);
        i_2 = rand_lim(CODEBOOK_SIZE_X);
        // randomly choose two indices between 0 and FINAL_C_SIZE_Y-1
        j_1 = rand_lim(CODEBOOK_SIZE_Y);
        j_2 = rand_lim(CODEBOOK_SIZE_Y);

        // measure current energy
        pot_diff = energy();
        // swap i_1 <-> i_2 & j_1 <-> j_2 temporarily
        tmp = bin_cw_x[i_1];
        bin_cw_x[i_1] = bin_cw_x[i_2];
        bin_cw_x[i_2] = bin_cw_x[i_1];
        // find the difference in new state's energy from old energy
        pot_diff -= energy();
        
        // keep swap if energy drop
        // else keep swap with probability e^{-rise/T}
        if (pot_diff <= 0) {
            drop_count++;
        }
        else if ((rand() / (double) RAND_MAX) < exp(-pot_diff/T)) {
            fail_count++;
        }
        else {
            fail_count++;
            // don't keep swap i.e. swap back
            tmp = bin_cw_x[i_1];
            bin_cw_x[i_1] = bin_cw_x[i_2];
            bin_cw_x[i_2] = bin_cw_x[i_1];
        }

        iter++;

        if (drop_count == phi || fail_count == psi) {
            // lower the temperature
            T *= cooling_rate;
            // reset counts
            drop_count = 0;
            fail_count = 0;
        }
    } while (T > T_final && iter < max_iterations);

}
