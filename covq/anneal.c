#include "anneal.h"
#define POW2(x) ((x)*(x))

/* Helper function to swap integers *i and *j */
void swap(uint *i, uint *j) {
    uint tmp;
    tmp = *i;
    *i = *j;
    *j = tmp;
}

/* get energy of current binary index assignment */
double energy(vectorset *codebook, size_t *count, uint *cw_map,
              size_t trset_size, double error_prob) {
    uint i, j;
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
uint rand_lim(uint limit) {
    uint divisor = RAND_MAX/(limit);
    uint retval;

    do { 
        retval = rand() / divisor;
    } while (retval >= limit);

    return retval;
}

void anneal(vectorset *codebook, size_t *count, uint *cw_map, size_t trset_size,
            double error_prob) {
    if (codebook->size == 1) {
        /* Save a bit of time */
        return;
    }
    double new_energy, old_energy;
    double T = TEMP_INIT;
    uint *best_cw_map = malloc(codebook->size * sizeof(uint));
    uint drop_count = 0, fail_count = 0;
    uint i, j;

    old_energy = energy(codebook, count, cw_map, trset_size, error_prob);
    for (uint k = 0; k < codebook->size; k++) {
        best_cw_map[k] = cw_map[k];
    }

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
        if (new_energy < old_energy) {
            old_energy = new_energy;
            /* Remember the state with lowest energy */
            for (uint k = 0; k < codebook->size; k++) {
                best_cw_map[k] = cw_map[k];
            }
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
    /* Restore the state with lowest energy */
    for (uint k = 0; k < codebook->size; k++) {
        cw_map[k] = best_cw_map[k];
    }
    free(best_cw_map);
}

int test_anneal(vectorset *codebook, size_t *count, uint *cw_map,
            size_t trset_size, double error_prob) {
    /* Test that resulting index map is a permutation */
    uint *orig_map = malloc(sizeof(uint) * codebook->size);
    for (size_t i = 0; i < codebook->size; i++) {
        orig_map[i] = cw_map[i];
    }
    anneal(codebook, count, cw_map, trset_size, error_prob);
    for (size_t i = 0; i < codebook->size; i++) {
        for (size_t j = 0; j < codebook->size; j++) {
            if (cw_map[i] == orig_map[j]) {
                break;
            }
            else if (j == codebook->size - 1) {
                fprintf(stderr, "%u not in new map\n", cw_map[i]);
                fprintf(stderr, "original cw_map not a permutation of new cw_map\n");
                fprintf(stderr, "orig: ");
                for (size_t k = 0; k < codebook->size; k++) {
                    fprintf(stderr, "%u%s", orig_map[k], (k < codebook->size-1)
                        ? IO_DELIM: "\n");
                }
                fprintf(stderr, "new: ");
                for (size_t k = 0; k < codebook->size; k++) {
                    fprintf(stderr, "%u%s", cw_map[k], (k < codebook->size-1)
                        ? IO_DELIM: "\n");
                }
                free(orig_map);
                assert(0);
            }
        }
    }
    free(orig_map);
    return 1;
}
