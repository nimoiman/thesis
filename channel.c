#include "channel.h"

/* Uniform(0,1] */
double drand() {
    return (rand() + 1.0) / (RAND_MAX + 1.0)
}

void box_muller(double mean, double var) {
    double r_1, r_2; // independent Uniform (0,1)
    r_1 = drand();
    // srand((int) r_1); // reset seed
    r_2 = drand();
    return sqrt(var)*(sqrt(-2*log(r_1))*cos(2*PI*r_2)) + mean;
}

void gaussian_channel(vectorset *vectors, int size, double mean, double var) {
    int i;
    for (i = 0; i < size; i++) {
        vectors[i] += box_muller(mean, var);
    }
}