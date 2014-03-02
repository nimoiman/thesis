#include "channel.h"
/* Defines functions which model / simulate a Gaussian channel
   (Binary Symmetric Channel) */

/* Uniform(0,1] */
double drand() {
    return (rand() + 1.0) / (RAND_MAX + 1.0);
}

double box_muller(double mean, double var) {
    double r_1, r_2; // independent Uniform (0,1)
    r_1 = drand();
    r_2 = drand();
    return sqrt(var)*(sqrt(-2*log(r_1))*cos(2*M_PI*r_2)) + mean;
}

void gaussian_channel(double *vector, int dim, double mean, double var) {
    int i;
    for (i = 0; i < dim; i++) {
        vector[i] += box_muller(mean, var);
    }
}

/* Return 1 on "success", 0 on "failure" */
int bernoulli(double p) {
    return (rand() < p * RAND_MAX) ? 1 : 0;
}

void binary_symmetric_channel(int *index, double error_prob,
                              int fixed_length) {
    int i;
    char *ch_index = (char*) index; // go through byte at a time
    for (i = 0; i < fixed_length; i++) {
        if (bernoulli(error_prob)) {
            // flip the bit
            *ch_index ^= (1 << (i % 8));
        }
        if (i % 8 == 7) {
            // next byte
            ch_index++;
        }
    }
}

/* Return probability of channel index cw_map[i] transitioning to cw_map[j] 
 * over BSC with epsilon = error_prob
 */
double channel_prob(int i, int j, double error_prob, int length,
                              int *cw_map) {
    int d = hamming_distance(cw_map[i], cw_map[j]);
    double prob = pow(error_prob, d) * pow(1 - error_prob, length - d);

    assert(d >= 0);
    assert(prob >= 0);
    return prob;
}

/* Nearest Neighbour encode vector, return channel index */
int test_encode(double *vector, vectorset *codebook, int *cw_map) {
    int i, channel_index = -1;
    double d = DBL_MAX;
    double d_new;
    for (i = 0; i < codebook->size; i++) {
        d_new = dist(vector, codebook->v[i], codebook->dim);
        if (d_new < d) {
            d = d_new;
            channel_index = cw_map[i];
        }
    }
    assert(channel_index >= 0);
    assert(channel_index < codebook->size);
    return channel_index;
}

/* Decode to the vector indexed by cw_map[received_index] */
int test_decode(int received_index, vectorset *codebook, int *cw_map) {
    int i;
    // find this channel index in cw_map, to find source index...
    for (i = 0; i < codebook->size; i++) {
        if (cw_map[i] == received_index) {
            // this is it
            break;
        }
    }
    return i;
}

double run_test(vectorset *codebook, int *cw_map, vectorset *test_set,
                double error_prob) {
    int i, channel_index;
    double distortion = 0;
    int error_count = 0;
    int reconstruction_index = 0;

    for (i = 0; i < test_set->size; i++) {
        // encode source index to nearest neighbour
        channel_index = test_encode(test_set->v[i], codebook, cw_map);

        // pass channel_index through BSC
        binary_symmetric_channel(&channel_index, error_prob,
            log2(codebook->size));

        // decode received index to codevector
        reconstruction_index = test_decode(channel_index, codebook, cw_map);

        if (channel_index != cw_map[reconstruction_index]) {
            error_count++;
        }

        // sum the end-to-end distortion
        distortion += dist(test_set->v[i], codebook->v[reconstruction_index],
            test_set->dim);
    }

    // printf("error_count = %d\n", error_count);
    distortion /= test_set->size;
    return distortion;
}
