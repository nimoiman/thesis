#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef __cplusplus
extern "C"{
#endif

#include <math.h> /* log, sqrt */
#include <stdlib.h> /* rand, srand, RAND_MAX */
#include <time.h> /* time (for rand seed) */
#include <stdio.h>

#define BSC_ERROR_PROB 0.1

double drand();
double box_muller(double mean, double var);
void gaussian_channel(double *vector, int dim, double mean, double var);
double transition_probability(int i, int j, double error_prob, int length);

#ifdef __cplusplus
}
#endif

#endif // CHANNEL_H
