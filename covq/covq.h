#ifndef COVQ_H
#define COVQ_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
#include <float.h> /* DBL_MAX */
#include <stdlib.h> /* malloc, free, size_t */
#include <math.h> /* log2 */
#include "vector.h" /* dist */
#include "channel.h" /* channel_prob */
#include "anneal.h" /* anneal */

#define LBG_EPS 0.001
#define CODE_VECTOR_DISPLACE 0.001

vectorset *bsc_covq(vectorset *train, int *cw_map, int n_splits,
                    double error_prob);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

