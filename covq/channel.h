#ifndef CHANNEL_H
#define CHANNEL_H

#ifdef __cplusplus
extern "C"{
#endif

#include <math.h> /* log, sqrt */
#include <stdlib.h> /* rand, srand, RAND_MAX */
#include <time.h> /* time (for rand seed) */
#include <stdio.h>
#include <float.h> /* DBL_MAX */
#include <assert.h> /* assert */
#include "vector.h" /* vectorset, dist */

double channel_prob(int i, int j, double error_prob, int length, int *cw_map);
double run_test(vectorset *codebook, int *cw_map, vectorset *test_set,
                double error_prob);

#ifdef __cplusplus
}
#endif

#endif // CHANNEL_H
