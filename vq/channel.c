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
    return rand() < p * RAND_MAX ? 1 : 0;
}

void binary_symmetric_channel(char *index, double error_prob,
                              int fixed_length) {
    int i;
    for (i = 0; i < fixed_length; i++) {
        if (bernoulli(error_prob)) {
            // flip the bit
            *index ^= (1 << (i % 8));
        }
        if (i % 8 == 7) {
            // next byte
            index++;
        }
    }
}

double transition_probability(int i, int j, double error_prob, int length) {
    return pow(error_prob, hamming_distance(i, j)) *
        pow(1 - error_prob, length - hamming_distance(i, j));
}
