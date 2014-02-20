#include "covq_2.h"

/* Helper function to simulated annealing
 * Return distance between codevectors represented by the two
 * respective pairs (x_1, y_1), (x_2, y_2) */
double eucl_dist(int x_1, int x_2, int y_1, int y_2) {
    return pow(c_x[x_1][y_1] - c_y[x_2][y_2], 2);
}

/* get energy difference of current codebook with index i and j swapped
 * for src for simulated annealing process 
 * positive value means energy went up */
double potential_diff(int x_1, int x_2, int y_1, int y_2) {
    int k, el;
    double diff = 0;
    for (k = 0; k < FINAL_C_SIZE_X; k++) {
        for (el = 0; el < FINAL_C_SIZE_Y; el++) {
            diff -= trans_prob(x_1, k, SRC_X) * trans_prob(y_1, el, SRC_Y) *
                eucl_dist(x_1, k, y_1, el);
            diff += trans_prob(x_1, k, SRC_X) * trans_prob(y_1, el, SRC_Y) *
                eucl_dist(x_2, k, y_2, el);
            diff -= trans_prob(x_2, k, SRC_X) * trans_prob(y_2, el, SRC_Y) *
                eucl_dist(x_2, k, y_2, el);
            diff += trans_prob(x_2, k, SRC_X) * trans_prob(y_2, el, SRC_Y) *
                eucl_dist(x_1, k, y_1, el);
        }
    }
    return diff;
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
    double T = 10.0, cooling_rate = 0.8, T_final = 0.00025;
    int iter = 0, max_iterations = 50000;
    int phi = 5, drop_count = 0, psi = 200, fail_count = 0;
    
    int x_1, x_2, y_1, y_2;
    do {
        // randomly choose two indices between 0 and FINAL_C_SIZE_X-1
        x_1 = rand_lim(FINAL_C_SIZE_X);
        x_2 = rand_lim(FINAL_C_SIZE_X);
        // randomly choose two indices between 0 and FINAL_C_SIZE_Y-1
        y_1 = rand_lim(FINAL_C_SIZE_Y);
        y_2 = rand_lim(FINAL_C_SIZE_Y);
        // measure energy drop/rise from swap
        pot_diff = potential_diff(x_1, x_2, y_1, y_2);
        
        // swap if energy drop
        // else swap with probability e^{-rise/T}
        if (pot_diff <= 0) {
            drop_count++;
            tmp = c_x[x_1][y_1];
            c_x[x_1][y_1] = c_x[x_2][y_2];
            c_x[x_2][y_2] = tmp;
        }
        else if ((rand() / (double) RAND_MAX) < exp(-pot_diff/T)) {
            fail_count++;
            tmp = c_x[x_1][y_1];
            c_x[x_1][y_1] = c_x[x_2][y_2];
            c_x[x_2][y_2] = tmp;
        }
        else {
            fail_count++;
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