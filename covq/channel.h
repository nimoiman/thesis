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
#include "io.h" /* print_vectorset */

double channel_prob(uint i, uint j, double error_prob, uint length, uint *cw_map);
double run_test(vectorset *codebook, uint *cw_map, vectorset *test_set,
                char *test_out_csv, double error_prob);

#ifdef __cplusplus
}
#endif

#endif // CHANNEL_H
