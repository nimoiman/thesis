#include "covq_2.h"
#include <math.h> /* log, sqrt, M_PI*/

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

int main(int argc, char const *argv[]) {
    int i;
    covq *c = malloc(sizeof(covq));
    c->tr_size = 500;
    c->tr_x = malloc(c->tr_size * sizeof(double));
    c->tr_y = malloc(c->tr_size * sizeof(double));
    // generate training data to identical Gaussian(0,10) distributions
    for (i = 0; i < c->tr_size; i++) {
        gaussian_channel(&(c->tr_x[i]), 1, 0, 10);
        gaussian_channel(&(c->tr_y[i]), 1, 0, 10);
    }
    // set mean and sigma on c
    c->mean_x[0] = 0;
    c->mean_y[0] = 0;
    c->sigma_x[0] = sqrt(10);
    c->sigma_y[0] = sqrt(10);
    bsc_2_source_covq(c);

    free(c->tr_x);
    free(c->tr_y);

    return 0;
}