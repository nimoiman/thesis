#include "anneal.h"
#define POW2(x) ((x)*(x))

/* Helper function to swap integers *i and *j */
void swap(int *i, int *j) {
    int tmp;
    tmp = *i;
    *i = *j;
    *j = tmp;
}

/* get energy of current binary index assignment */
double energy(vectorset *codebook, int *count, int *cw_map, int trset_size,
              double error_prob) {
    int i, j, k, el;
    double eucl_dist;
    double sum = 0;
    double inner_sum;

    for (i = 0; i < codebook->size; i++) {
        inner_sum = 0;
        for (j = 0; j < codebook->size; j++) {
            eucl_dist = POW2(codebook->v[i] - codebook->v[j]);
            inner_sum += channel_prob(i, j, error_prob,
                log2(codebook->size), cw_map) * eucl_dist;
        }
        assert(inner_sum >= 0);
        sum += inner_sum * count[i];
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

void anneal(vectorset *codebook, int *count, int *cw_map, int trset_size,
            double error_prob) {
    double new_energy, old_energy;
    double T = TEMP_INIT;
    int drop_count = 0, fail_count = 0;
    int i, j;

    old_energy = energy(codebook, count, cw_map, trset_size, error_prob);

    while (T > TEMP_FINAL) {
        // randomly choose two indices in [0, codebook->size-1]
        i = rand_lim(codebook->size);
        j = rand_lim(codebook->size);

        // swap i <-> j temporarily
        swap(cw_map + i, cw_map + j);

        // find the difference in new state's energy from old energy
        new_energy = energy(codebook, count, cw_map, trset_size, error_prob);
        
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
            swap(cw_map + i, cw_map + j);
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
