#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef __cplusplus
extern "C"{
#endif

#include <math.h> /* log, sqrt */
#include <stdlib.h> /* rand, srand, RAND_MAX */
#include <time.h> /* time (for rand seed) */
#include <stdio.h>

double drand();
double box_muller(double mean, double var);
void gaussian_channel(double *vector, int dim, double mean, double var);

#ifdef __cplusplus
}
#endif

#endif // CHANNEL_H  xsss